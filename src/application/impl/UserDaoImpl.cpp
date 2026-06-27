// UserDaoImpl.cpp - UserDao 的 MySQL 实现
#include "UserDaoImpl.h"
#include "Log.h"
#include "LogModule.h"
#include "MySqlMgr.h"

#include <chrono>
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
    bool exists = rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE name = ? LIMIT 1", name);
    LOGD(LogModule::Mysql, "userNameExists name={} exists={}", name, exists);
    return exists;
}

bool UserDaoImpl::emailExists(const std::string &email)
{
    bool exists = rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE email = ? LIMIT 1", email);
    LOGD(LogModule::Mysql, "emailExists email={} exists={}", email, exists);
    return exists;
}

int UserDaoImpl::regUser(const std::string &name, const std::string &email, const std::string &pwd,
                         const std::string &nick, int sex)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Mysql, "regUser start name={} email={}", name, email);

    int uid = -1;
    bool ok = _mysql_mgr->withConn([&](sql::Connection &conn) {
        if (rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE name = ? LIMIT 1", name) ||
            rowExists(*_mysql_mgr, "SELECT 1 FROM user WHERE email = ? LIMIT 1", email))
        {
            uid = 0;
            LOGW(LogModule::Mysql, "regUser user already exists name={} email={}", name, email);
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
            LOGE(LogModule::Mysql, "regUser failed to fetch new uid from user_id table");
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

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (ok && uid > 0)
    {
        LOGI(LogModule::Mysql, "regUser success uid={} name={} cost={}ms", uid, name, cost_ms);
    }
    else
    {
        LOGE(LogModule::Mysql, "regUser failed uid={} name={} cost={}ms", uid, name, cost_ms);
    }
    return uid;
}

bool UserDaoImpl::checkEmail(const std::string &email, const std::string &name)
{
    bool ok = _mysql_mgr->queryOne(
        "SELECT name FROM user WHERE email = ?",
        [&](sql::PreparedStatement &stmt) {
            stmt.setString(1, email);
        },
        [&](sql::ResultSet &rs) {
            return rs.getString("name") == name;
        });
    LOGD(LogModule::Mysql, "checkEmail email={} name={} match={}", email, name, ok);
    return ok;
}

bool UserDaoImpl::updatePwd(const std::string &email, const std::string &pwd)
{
    int rows = _mysql_mgr->exec("UPDATE user SET pwd = ? WHERE email = ?",
                                [&](sql::PreparedStatement &stmt) {
                                    stmt.setString(1, pwd);
                                    stmt.setString(2, email);
                                });
    bool ok = rows > 0;
    if (ok)
    {
        LOGI(LogModule::Mysql, "updatePwd success email={} rows={}", email, rows);
    }
    else
    {
        LOGE(LogModule::Mysql, "updatePwd failed email={} rows={}", email, rows);
    }
    return ok;
}

bool UserDaoImpl::checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo)
{
    bool matched = false;
    bool ok = _mysql_mgr->queryOne(
        "SELECT uid, name, pwd FROM user WHERE email = ?",
        [&](sql::PreparedStatement &stmt) {
            stmt.setString(1, email);
        },
        [&](sql::ResultSet &rs) {
            if (rs.getString("pwd") != pwd)
            {
                LOGI(LogModule::Mysql, "checkPwd password mismatch email={}", email);
                return false;
            }
            userInfo.uid = rs.getInt("uid");
            userInfo.name = rs.getString("name");
            userInfo.email = email;
            userInfo.pwd = rs.getString("pwd");
            matched = true;
            return true;
        });

    if (ok && matched)
    {
        LOGI(LogModule::Mysql, "checkPwd success uid={} email={}", userInfo.uid, email);
    }
    else if (ok && !matched)
    {
        LOGI(LogModule::Mysql, "checkPwd no match for email={}", email);
    }
    else
    {
        LOGE(LogModule::Mysql, "checkPwd query failed email={}", email);
    }
    return matched;
}
