// MySqlMgr.cpp - 数据库访问通用封装
#include "MySqlMgr.h"
#include "DbSession.h"
#include "Log.h"
#include "LogModule.h"

#include <chrono>

MySqlMgr::MySqlMgr()
{
    MySqlPool::initOnce();
}

bool MySqlMgr::withConn(std::function<bool(sql::Connection &)> fn)
{
    return DbSession::withConn(std::move(fn));
}

int MySqlMgr::exec(const std::string &sql, BindFn bind)
{
    const auto start = std::chrono::steady_clock::now();
    int rows = -1;
    bool ok = withConn([&](sql::Connection &conn) {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
        if (bind)
        {
            bind(*stmt);
        }
        rows = stmt->executeUpdate();
        return rows >= 0;
    });

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (ok)
    {
        LOGD(LogModule::Mysql, "exec ok, rows={} cost={}ms sql={}", rows, cost_ms, sql);
    }
    else
    {
        LOGE(LogModule::Mysql, "exec failed, rows={} cost={}ms sql={}", rows, cost_ms, sql);
    }
    return rows;
}

bool MySqlMgr::queryOne(const std::string &sql, BindFn bind,
                        std::function<bool(sql::ResultSet &)> map)
{
    const auto start = std::chrono::steady_clock::now();
    bool found = false;
    bool ok = withConn([&](sql::Connection &conn) {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
        if (bind)
        {
            bind(*stmt);
        }
        auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        if (!rs->next())
        {
            return true;
        }
        found = true;
        return map(*rs);
    });

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (ok)
    {
        LOGD(LogModule::Mysql, "queryOne ok, found={} cost={}ms sql={}", found, cost_ms, sql);
    }
    else
    {
        LOGE(LogModule::Mysql, "queryOne failed, found={} cost={}ms sql={}", found, cost_ms,
             sql);
    }
    return ok;
}
