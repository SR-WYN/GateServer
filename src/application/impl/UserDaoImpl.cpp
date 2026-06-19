// UserDaoImpl.cpp - UserDao 的 MySQL 实现
#include "UserDaoImpl.h"
#include "MySqlMgr.h"
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <memory>

UserDaoImpl::UserDaoImpl(std::shared_ptr<MySqlMgr> mysql_mgr)
    : _mysql_mgr(std::move(mysql_mgr))
{
}

namespace
{
bool rowExists(MySqlMgr &mgr, const std::string &sql, const std::string &value)
{
    return mgr.queryOne(
        sql,
        [&](sql::PreparedStatement &stmt) {
            stmt.setString(1, value);
        },
        [](sql::ResultSet &) {
            return true;
        });
}
} // namespace

bool UserDaoImpl::userNameExists(const std::string &name)
{
    return rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE name = ? LIMIT 1", name);
}

bool UserDaoImpl::emailExists(const std::string &email)
{
    return rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE email = ? LIMIT 1", email);
}

int UserDaoImpl::regUser(const std::string &name, const std::string &email, const std::string &pwd,
                         const std::string &nick, int sex)
{
    int uid = -1;
    _mysql_mgr->withConn([&](sql::Connection &conn) {
        if (rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE name = ? LIMIT 1", name) ||
            rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE email = ? LIMIT 1", email))
        {
            uid = 0;
            return true;
        }

        conn.setAutoCommit(false);
        auto bump = std::unique_ptr<sql::Statement>(conn.createStatement());
        bump->executeUpdate("UPDATE user_id SET id = id + 1");

        auto fetchId = std::unique_ptr<sql::Statement>(conn.createStatement());
        auto idRes = std::unique_ptr<sql::ResultSet>(
            fetchId->executeQuery("SELECT id FROM user_id LIMIT 1"));
        if (!idRes->next())
        {
            conn.rollback();
            return false;
        }
        const int new_uid = idRes->getInt("id");

        auto insert = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(
            "INSERT INTO user (uid, name, nick, `desc`, sex, icon, email, pwd) "
            "VALUES (?,?,?,?,?,?,?,?)"));
        insert->setInt(1, new_uid);
        insert->setString(2, name);
        insert->setString(3, nick);
        insert->setString(4, "");
        insert->setInt(5, sex);
        insert->setString(6, "");
        insert->setString(7, email);
        insert->setString(8, pwd);
        insert->executeUpdate();
        conn.commit();
        uid = new_uid;
        return true;
    });
    return uid;
}

bool UserDaoImpl::checkEmail(const std::string &email, const std::string &name)
{
    return _mysql_mgr->queryOne(
        "SELECT name FROM user WHERE email = ?",
        [&](sql::PreparedStatement &stmt) {
            stmt.setString(1, email);
        },
        [&](sql::ResultSet &rs) {
            return rs.getString("name") == name;
        });
}

bool UserDaoImpl::updatePwd(const std::string &email, const std::string &pwd)
{
    return _mysql_mgr->exec("UPDATE user SET pwd = ? WHERE email = ?",
                            [&](sql::PreparedStatement &stmt) {
                                stmt.setString(1, pwd);
                                stmt.setString(2, email);
                            }) > 0;
}

bool UserDaoImpl::checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo)
{
    bool matched = false;
    _mysql_mgr->queryOne(
        "SELECT uid, name, pwd FROM user WHERE email = ?",
        [&](sql::PreparedStatement &stmt) {
            stmt.setString(1, email);
        },
        [&](sql::ResultSet &rs) {
            if (rs.getString("pwd") != pwd)
            {
                return false;
            }
            userInfo.uid = rs.getInt("uid");
            userInfo.name = rs.getString("name");
            userInfo.email = email;
            userInfo.pwd = rs.getString("pwd");
            matched = true;
            return true;
        });
    return matched;
}
