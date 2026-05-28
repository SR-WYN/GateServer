#include "ConfigMgr.h"
#include "MySqlDao.h"
#include "MySqlPool.h"
#include "utils.h"
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <iostream>

MySqlDao::MySqlDao()
{
    auto& cfg = ConfigMgr::getInstance();
    const auto& host = cfg["MySql"]["Host"];
    const auto& port = cfg["MySql"]["Port"];
    const auto& user = cfg["MySql"]["User"];
    const auto& pass = cfg["MySql"]["Passwd"];
    const auto& schema = cfg["MySql"]["Schema"];
    _pool.reset(new MySqlPool(host + ":" + port, user, pass, schema, 5));
}

MySqlDao::~MySqlDao()
{
    _pool->close();
}

namespace
{
bool rowExists(sql::Connection* conn, const std::string& sql, const std::string& value)
{
    std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(sql));
    pstmt->setString(1, value);
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    return res->next();
}
} // namespace

bool MySqlDao::userNameExists(const std::string& name)
{
    auto con = _pool->getConnection();
    if (con == nullptr || con->_con == nullptr)
    {
        return false;
    }
    utils::Defer defer([this, &con]() { _pool->returnConnection(std::move(con)); });
    try
    {
        return rowExists(con->_con.get(), "SELECT 1 FROM user WHERE name = ? LIMIT 1", name);
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        return false;
    }
}

bool MySqlDao::emailExists(const std::string& email)
{
    auto con = _pool->getConnection();
    if (con == nullptr || con->_con == nullptr)
    {
        return false;
    }
    utils::Defer defer([this, &con]() { _pool->returnConnection(std::move(con)); });
    try
    {
        return rowExists(con->_con.get(), "SELECT 1 FROM user WHERE email = ? LIMIT 1", email);
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        return false;
    }
}

int MySqlDao::regUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    auto con = _pool->getConnection();
    if (con == nullptr || con->_con == nullptr)
    {
        return -1;
    }
    utils::Defer defer([this, &con]() { _pool->returnConnection(std::move(con)); });
    try
    {
        if (rowExists(con->_con.get(), "SELECT 1 FROM user WHERE name = ? LIMIT 1", name) ||
            rowExists(con->_con.get(), "SELECT 1 FROM user WHERE email = ? LIMIT 1", email))
        {
            return 0;
        }

        con->_con->setAutoCommit(false);

        std::unique_ptr<sql::Statement> bump(con->_con->createStatement());
        bump->executeUpdate("UPDATE user_id SET id = id + 1");

        std::unique_ptr<sql::Statement> fetchId(con->_con->createStatement());
        std::unique_ptr<sql::ResultSet> idRes(fetchId->executeQuery("SELECT id FROM user_id LIMIT 1"));
        if (!idRes->next())
        {
            con->_con->rollback();
            std::cerr << "user_id table is empty" << std::endl;
            return -1;
        }
        const int uid = idRes->getInt("id");

        std::unique_ptr<sql::PreparedStatement> insert(con->_con->prepareStatement(
            "INSERT INTO user (uid, name, nick, `desc`, sex, icon, email, pwd) "
            "VALUES (?,?,?,?,?,?,?,?)"));
        insert->setInt(1, uid);
        insert->setString(2, name);
        insert->setString(3, name); // 与 ChatServer 一致：注册时 nick 默认等于 name
        insert->setString(4, "");
        insert->setInt(5, 0);
        insert->setString(6, "");
        insert->setString(7, email);
        insert->setString(8, pwd);
        insert->executeUpdate();

        con->_con->commit();
        std::cout << "registered user uid=" << uid << " name=" << name << std::endl;
        return uid;
    }
    catch (sql::SQLException& e)
    {
        try
        {
            con->_con->rollback();
        }
        catch (...)
        {
        }
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MYSQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return -1;
    }
}

bool MySqlDao::checkEmail(const std::string& email, const std::string& name)
{
    auto con = _pool->getConnection();
    try
    {
        if (con == nullptr)
        {
            return false;
        }

        // 准备查询语句
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->_con->prepareStatement("SELECT name FROM user WHERE email = ?"));
        // 绑定参数
        pstmt->setString(1, email);
        // 执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next())
        {
            std::cout << "Check name: " << res->getString("name") << std::endl;
            if (name == res->getString("name"))
            {
                _pool->returnConnection(std::move(con));
                return true;
            }
        }
        _pool->returnConnection(std::move(con));
        return false;
    }
    catch (sql::SQLException& e)
    {
        _pool->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MYSQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return false;
    }
}

bool MySqlDao::updatePwd(const std::string& email, const std::string& pwd)
{
    auto con = _pool->getConnection();
    try
    {
        if (con == nullptr)
        {
            return false;
        }
        // 准备查询语句
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->_con->prepareStatement("UPDATE user SET pwd = ? WHERE email = ?"));
        // 绑定参数
        pstmt->setString(1, pwd);
        pstmt->setString(2, email);
        // 执行更新
        int updateCount = pstmt->executeUpdate();

        std::cout << "Updated rows: " << updateCount << std::endl;
        _pool->returnConnection(std::move(con));
        return updateCount > 0;
    }
    catch (sql::SQLException& e)
    {
        _pool->returnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MYSQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return false;
    }
}

bool MySqlDao::checkPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo)
{
    auto con = _pool->getConnection();
    if (con == nullptr)
    {
        return false;
    }
    utils::Defer defer([this,&con](){
        _pool->returnConnection(std::move(con));
    });
    try 
    {
        //准备SQL语句
        if (con->_con == nullptr)
        {
            std::cerr << "Connection is nullptr" << std::endl;
            return false;
        }
        std::unique_ptr<sql::PreparedStatement> pstmt(con->_con->prepareStatement("SELECT * FROM user WHERE email = ?"));
        pstmt->setString(1,email);

        //执行查询
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::string origin_pwd = "";
        if (res->next())
        {
            origin_pwd = res->getString("pwd");
            // 输出查询到的密码
            std::cout << "Password: " << origin_pwd << std::endl;
        }
        if (pwd != origin_pwd)
        {
            return false;
        }
        userInfo.name = res->getString("name");
        userInfo.email = email;
        userInfo.pwd = origin_pwd;
        userInfo.uid = res->getInt("uid");
        return true;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MYSQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return false;
    }
}