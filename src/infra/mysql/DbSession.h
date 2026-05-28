#pragma once

#include "MySqlPool.h"
#include "utils.h"
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

class DbSession
{
public:
    using BindFn = std::function<void(sql::PreparedStatement &)>;

    template <typename Fn>
    static bool withConn(MySqlPool &pool, Fn &&fn)
    {
        auto con = pool.getConnection();
        if (!con || !con->_con)
        {
            return false;
        }
        utils::Defer defer([&]() { pool.returnConnection(std::move(con)); });
        try
        {
            return fn(*con->_con);
        }
        catch (sql::SQLException &e)
        {
            return false;
        }
    }

    static int exec(MySqlPool &pool, const std::string &sql, BindFn bind = nullptr)
    {
        int rows = -1;
        withConn(pool, [&](sql::Connection &conn) {
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

    template <typename MapFn>
    static bool queryOne(MySqlPool &pool, const std::string &sql, BindFn bind, MapFn &&map)
    {
        return withConn(pool, [&](sql::Connection &conn) {
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
};
