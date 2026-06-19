// MySqlMgr.cpp - 数据库访问通用封装
#include "MySqlMgr.h"
#include "DbSession.h"

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
    int rows = -1;
    withConn([&](sql::Connection &conn) {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
        if (bind)
        {
            bind(*stmt);
        }
        rows = stmt->executeUpdate();
        return rows >= 0;
    });
    return rows;
}

bool MySqlMgr::queryOne(const std::string &sql, BindFn bind,
                        std::function<bool(sql::ResultSet &)> map)
{
    return withConn([&](sql::Connection &conn) {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
        if (bind)
        {
            bind(*stmt);
        }
        auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        if (!rs->next())
        {
            return false;
        }
        return map(*rs);
    });
}
