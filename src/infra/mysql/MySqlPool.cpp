#include "MySqlPool.h"
#include "ConfigMgr.h"
#include "utils.h"
#include <boost/asio/steady_timer.hpp>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <memory>
#include <mutex>
#include <mysql_driver.h>

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
            auto currentTime = std::chrono::system_clock::now().time_since_epoch();
            long long timestamp =
                std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
            _pool.push(std::make_unique<SqlConnection>(con, timestamp));
        }
    }
    catch (sql::SQLException &e)
    {
    }
}

void MySqlPool::startHealthCheck(boost::asio::io_context &ioc)
{
    auto timer = std::make_shared<boost::asio::steady_timer>(ioc);

    std::function<void()> doCheck;
    doCheck = [this, timer, &doCheck]() {
        if (_stop)
            return;
        checkConnection();
        timer->expires_after(std::chrono::seconds(60));
        timer->async_wait([&doCheck](const boost::system::error_code &ec) {
            if (!ec)
                doCheck();
        });
    };
    // 立即执行第一次检查，然后每 60s 一次
    doCheck();
}

MySqlPool::~MySqlPool()
{
    close();

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

    auto con = std::move(_pool.front());
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
    auto currentTime = std::chrono::system_clock::now().time_since_epoch();
    con->_last_oper_time =
        std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
    _pool.push(std::move(con));
}

void MySqlPool::close()
{
    _stop.store(true);
    _cond.notify_all();
}

void MySqlPool::checkConnection()
{
    std::lock_guard<std::mutex> lock(_mutex);
    int poolSize = _pool.size();
    for (int i = 0; i < poolSize; i++)
    {
        auto con = std::move(_pool.front());
        _pool.pop();
        auto currentTime = std::chrono::system_clock::now().time_since_epoch();
        long long timestamp =
            std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
        if (timestamp - con->_last_oper_time < 60)
        {
            _pool.push(std::move(con));
            continue;
        }
        if (reconnect(timestamp))
        {
            _fail_count.store(0);
            _pool.push(std::move(con));
        }
    }
}

bool MySqlPool::reconnect(long long timestamp)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_fail_count.load() >= 3)
        {
            return false;
        }
    }
    try
    {
        sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
        auto *con = driver->connect(_url, _user, _pass);
        con->setSchema(_schema);
        auto p = std::make_unique<SqlConnection>(con, timestamp);
        _pool.push(std::move(p));
        return true;
    }
    catch (sql::SQLException &e)
    {
        _fail_count.fetch_add(1);
        return false;
    }
}