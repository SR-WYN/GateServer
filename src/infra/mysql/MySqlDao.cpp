#include "ConfigMgr.h"
#include "MySqlDao.h"
#include "DbSession.h"
#include "MySqlPool.h"
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>

MySqlDao::MySqlDao()
{
    auto &cfg = ConfigMgr::getInstance();
    const auto &host = cfg["MySql"]["Host"];
    const auto &port = cfg["MySql"]["Port"];
    const auto &user = cfg["MySql"]["User"];
    const auto &pass = cfg["MySql"]["Passwd"];
    const auto &schema = cfg["MySql"]["Schema"];
    _pool.reset(new MySqlPool(host + ":" + port, user, pass, schema, 5));
}

MySqlDao::~MySqlDao()
{
    _pool->close();
}

namespace
{
bool rowExists(sql::Connection &conn, const std::string &sql, const std::string &value)
{
    auto stmt = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(sql));
    stmt->setString(1, value);
    auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
    return rs->next();
}
} // namespace

bool MySqlDao::userNameExists(const std::string &name)
{
    return DbSession::queryOne(
        *_pool, "SELECT 1 FROM user WHERE name = ? LIMIT 1",
        [&](sql::PreparedStatement &stmt) { stmt.setString(1, name); },
        [](sql::ResultSet &) { return true; });
}

bool MySqlDao::emailExists(const std::string &email)
{
    return DbSession::queryOne(
        *_pool, "SELECT 1 FROM user WHERE email = ? LIMIT 1",
        [&](sql::PreparedStatement &stmt) { stmt.setString(1, email); },
        [](sql::ResultSet &) { return true; });
}

int MySqlDao::regUser(const std::string &name, const std::string &email, const std::string &pwd)
{
    int uid = -1;
    DbSession::withConn(*_pool, [&](sql::Connection &conn) {
        if (rowExists(conn, "SELECT 1 FROM user WHERE name = ? LIMIT 1", name) ||
            rowExists(conn, "SELECT 1 FROM user WHERE email = ? LIMIT 1", email))
        {
            uid = 0;
            return true;
        }

        conn.setAutoCommit(false);
        auto bump = std::unique_ptr<sql::Statement>(conn.createStatement());
        bump->executeUpdate("UPDATE user_id SET id = id + 1");

        auto fetchId = std::unique_ptr<sql::Statement>(conn.createStatement());
        auto idRes = std::unique_ptr<sql::ResultSet>(fetchId->executeQuery("SELECT id FROM user_id LIMIT 1"));
        if (!idRes->next())
        {
            conn.rollback();
            std::cerr << "user_id table is empty" << std::endl;
            return false;
        }
        const int new_uid = idRes->getInt("id");

        auto insert = std::unique_ptr<sql::PreparedStatement>(conn.prepareStatement(
            "INSERT INTO user (uid, name, nick, `desc`, sex, icon, email, pwd) "
            "VALUES (?,?,?,?,?,?,?,?)"));
        insert->setInt(1, new_uid);
        insert->setString(2, name);
        insert->setString(3, name);
        insert->setString(4, "");
        insert->setInt(5, 0);
        insert->setString(6, "");
        insert->setString(7, email);
        insert->setString(8, pwd);
        insert->executeUpdate();
        conn.commit();
        uid = new_uid;
        std::cout << "registered user uid=" << uid << " name=" << name << std::endl;
        return true;
    });
    return uid;
}

bool MySqlDao::checkEmail(const std::string &email, const std::string &name)
{
    return DbSession::queryOne(
        *_pool, "SELECT name FROM user WHERE email = ?",
        [&](sql::PreparedStatement &stmt) { stmt.setString(1, email); },
        [&](sql::ResultSet &rs) { return rs.getString("name") == name; });
}

bool MySqlDao::updatePwd(const std::string &email, const std::string &pwd)
{
    return DbSession::exec(
               *_pool, "UPDATE user SET pwd = ? WHERE email = ?",
               [&](sql::PreparedStatement &stmt) {
                   stmt.setString(1, pwd);
                   stmt.setString(2, email);
               }) > 0;
}

bool MySqlDao::checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo)
{
    bool matched = false;
    DbSession::queryOne(
        *_pool, "SELECT uid, name, pwd FROM user WHERE email = ?",
        [&](sql::PreparedStatement &stmt) { stmt.setString(1, email); },
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
