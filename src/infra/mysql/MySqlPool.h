#pragma once
#include "Singleton.h"
#include <boost/asio/io_context.hpp>
#include <atomic>
#include <condition_variable>
#include <cppconn/connection.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

class SqlConnection
{
public:
    SqlConnection(sql::Connection *con, int64_t lasttime);
    std::unique_ptr<sql::Connection> _con;
    int64_t _last_oper_time;
};

class MySqlPool : public Singleton<MySqlPool>
{
    friend class Singleton<MySqlPool>;

public:
    static void initOnce();
    ~MySqlPool() override;
    std::unique_ptr<SqlConnection> getConnection();
    void returnConnection(std::unique_ptr<SqlConnection> con);
    void close();
    void checkConnection();
    bool reconnect(long long timestamp);

    // 在指定 io_context 上启动定时健康检查（替代独立线程）
    void startHealthCheck(boost::asio::io_context &ioc);

private:
    MySqlPool();
    MySqlPool(const MySqlPool &) = delete;
    MySqlPool &operator=(const MySqlPool &) = delete;
    void initPool(const std::string &url, const std::string &user, const std::string &pass,
                  const std::string &schema, int poolSize);

    std::mutex _init_mutex;
    std::atomic<bool> _initialized{false};
    std::string _url;
    std::string _user;
    std::string _pass;
    std::string _schema;
    int _pool_size{0};
    std::queue<std::unique_ptr<SqlConnection>> _pool;
    std::mutex _mutex;
    std::condition_variable _cond;
    std::atomic<bool> _stop{false};
    std::atomic<int> _fail_count{0};
};