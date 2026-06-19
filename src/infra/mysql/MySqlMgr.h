// MySqlMgr.h - 数据库访问通用封装
#pragma once

#include "MySqlPool.h"
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

/// 数据库访问管理器
/// 提供基础 SQL 操作原语，上层业务 DAO 据此实现具体业务逻辑
class MySqlMgr
{
public:
    MySqlMgr();
    ~MySqlMgr() = default;

    using BindFn = std::function<void(sql::PreparedStatement &)>;

    /// 在连接池取出的连接上执行自定义操作
    bool withConn(std::function<bool(sql::Connection &)> fn);

    /// 执行 INSERT / UPDATE / DELETE，返回影响行数（失败返回 -1）
    int exec(const std::string &sql, BindFn bind = nullptr);

    /// 执行查询并映射首条记录，无记录返回 false
    bool queryOne(const std::string &sql, BindFn bind,
                  std::function<bool(sql::ResultSet &)> map);

    /// 执行查询并映射所有记录
    template <typename MapFn, typename T>
    bool queryAll(const std::string &sql, BindFn bind, MapFn &&map, std::vector<T> &out);
};

template <typename MapFn, typename T>
bool MySqlMgr::queryAll(const std::string &sql, BindFn bind, MapFn &&map, std::vector<T> &out)
{
    out.clear();
    return withConn([&](sql::Connection &conn) {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
        if (bind)
        {
            bind(*stmt);
        }
        auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        while (rs->next())
        {
            out.push_back(map(*rs));
        }
        return true;
    });
}
