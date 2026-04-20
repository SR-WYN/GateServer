#pragma once
#include <string>
#include <memory>

class MySqlPool;

class MySqlDao
{
public:
    MySqlDao();
    ~MySqlDao();
    int RegUser(const std::string& name,const std::string& email,const std::string& pwd);
    bool CheckEmail(const std::string& email,const std::string& name);
    bool UpdatePwd(const std::string& email,const std::string& pwd);
private:
    std::unique_ptr<MySqlPool> _pool;
};