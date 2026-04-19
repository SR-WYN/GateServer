#include "MySqlDao.h"
#include "ConfigMgr.h"
#include "MySqlPool.h"
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>

MySqlDao::MySqlDao()
{
    auto& cfg = ConfigMgr::GetInstance();
    const auto& host = cfg["MySql"]["Host"];
    const auto& port = cfg["MySql"]["Port"];
    const auto& user = cfg["MySql"]["User"];
    const auto& pass = cfg["MySql"]["Passwd"];
    const auto& schema = cfg["MySql"]["Schema"];
    _pool.reset(new MySqlPool(host + ":" + port, user, pass, schema, 5));
}

MySqlDao::~MySqlDao()
{
    _pool->Close();
}

int MySqlDao::RegUser(const std::string& name, const std::string& email, const std::string& pwd)
{
    auto con = _pool->GetConnection();
    try
    {
        if (con == nullptr)
        {
            return -1;
        }
        // 准备调用存储过程
        std::unique_ptr<sql::PreparedStatement> stmt(
            con->_con->prepareStatement("CALL reg_user(?,?,?,@result)"));
        // 设置输入参数
        stmt->setString(1, name);
        stmt->setString(2, email);
        stmt->setString(3, pwd);
        // 执行存储过程
        stmt->execute();
        std::unique_ptr<sql::Statement> stmtResult(con->_con->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmtResult->executeQuery("SELECT @result AS result"));
        if (res->next())
        {
            int result = res->getInt("result");
            std::cout << "Result: " << result << std::endl;
            _pool->ReturnConnection(std::move(con));
            return result;
        }
        _pool->ReturnConnection(std::move(con));
        return -1;
    }
    catch (sql::SQLException& e)
    {
        _pool->ReturnConnection(std::move(con));
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MYSQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
        return -1;
    }
}