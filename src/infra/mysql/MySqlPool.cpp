#include "MySqlPool.h"
#include "ConfigMgr.h"
#include "utils.h"
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <memory>
#include <mutex>
#include <mysql_driver.h>
#include <thread>

SqlConnection::SqlConnection(sql::Connection *con, int64_t lasttime)
    : _con(con), _last_oper_time(lasttime)
{
}

MySqlPool::MySqlPool() = default;

void MySqlPool::initOnce()
{
    auto &pool = getInstance();
    std::lock_guard<std::mutex> lock(pool._init_mutex);
    if (pool._initialized.load())
    {
        return;
    }
    auto &cfg = ConfigMgr::getInstance();
    const auto &host = cfg["MySql"]["Host"];
    const auto &port = cfg["MySql"]["Port"];
    const auto &user = cfg["MySql"]["User"];
    const auto &pass = cfg["MySql"]["Passwd"];
    const auto &schema = cfg["MySql"]["Schema"];
    pool.initPool(host + ":" + port, user, pass, schema, 5);
    pool._initialized.store(true);
}

void MySqlPool::initPool(const std::string &url, const std::string &user, const std::string &pass,
                         const std::string &schema, int poolSize)
{
    _url = url;
    _user = user;
    _pass = pass;
    _schema = schema;
    _pool_size = poolSize;
    _stop.store(false);
    _fail_count.store(0);
    try
    {
        for (int i = 0; i < _pool_size; i++)
        {
            sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
            auto *con = driver->connect(_url, _user, _pass);
            con->setSchema(_schema);
            // 获取当前时间戳
            auto currentTime = std::chrono::system_clock::now().time_since_epoch();
            // 将时间戳转换为秒
            long long timestamp =
                std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
            _pool.push(std::make_unique<SqlConnection>(con, timestamp));
        }
        _check_thread = std::thread([this]() {
            while (!_stop)
            {
                checkConnection();
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
        });
    }
    catch (sql::SQLException &e)
    {
    }
}

MySqlPool::~MySqlPool()
{
    close();
    if (_check_thread.joinable())
    {
        _check_thread.join();
    }

    std::lock_guard<std::mutex> lock(_mutex);
    while (!_pool.empty())
    {
        _pool.pop();
    }
}

std::unique_ptr<SqlConnection> MySqlPool::getConnection()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock, [this]() {
        if (_stop)
        {
            return true;
        }
        return !_pool.empty();
    });

    if (_stop)
    {
        return nullptr;
    }

    std::unique_ptr<SqlConnection> con(std::move(_pool.front()));
    _pool.pop();
    return con;
}

void MySqlPool::returnConnection(std::unique_ptr<SqlConnection> con)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_stop)
    {
        return;
    }
    _pool.push(std::move(con));
    _cond.notify_one();
}

void MySqlPool::close()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _stop = true;
    _cond.notify_all();
}

void MySqlPool::checkConnection()
{
    size_t targetCount;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        targetCount = _pool.size();
    }

    auto now = std::chrono::system_clock::now().time_since_epoch();
    long long timestamp = std::chrono::duration_cast<std::chrono::seconds>(now).count();

    for (size_t i = 0; i < targetCount; ++i)
    {
        std::unique_ptr<SqlConnection> con;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_pool.empty())
                break;
            con = std::move(_pool.front());
            _pool.pop();
        }

        bool healthy = true;
        utils::Defer defer([&] {
            if (healthy && con)
            {
                std::lock_guard<std::mutex> lock(_mutex);
                _pool.push(std::move(con));
                _cond.notify_one();
            }
        });

        // 检查活跃度
        if (timestamp - con->_last_oper_time >= 5)
        {
            try
            {
                std::unique_ptr<sql::Statement> stmt(con->_con->createStatement());
                stmt->executeQuery("SELECT 1");
                con->_last_oper_time = timestamp;
            }
            catch (sql::SQLException &e)
            {
                healthy = false; // 标记为坏连接，DEFER 此时不会将其还回池子
                _fail_count++;
            }
        }
    }

    // 重连补偿：确保把缺额补齐
    while (_fail_count > 0)
    {
        if (reconnect(timestamp))
        {
            _fail_count--;
        }
        else
        {
            break; // 确实连不上了（如网络断开），退出等下次检查
        }
    }
}

bool MySqlPool::reconnect(long long timestamp)
{
    try
    {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        auto *con = driver->connect(_url, _user, _pass);
        con->setSchema(_schema);

        auto newCon = std::make_unique<SqlConnection>(con, timestamp);
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _pool.push(std::move(newCon));
        }

        return true;
    }
    catch (sql::SQLException &e)
    {
        return false;
    }
}
