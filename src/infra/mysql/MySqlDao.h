#pragma once
#include "MySqlPool.h"
#include "data.h"
#include <memory>
#include <string>

class MySqlPool;

class MySqlDao
{
public:
    MySqlDao();
    ~MySqlDao();
    int regUser(const std::string &name, const std::string &email, const std::string &pwd,
                const std::string &nick, int sex);
    bool userNameExists(const std::string &name);
    bool emailExists(const std::string &email);
    bool checkEmail(const std::string &email, const std::string &name);
    bool updatePwd(const std::string &email, const std::string &pwd);
    bool checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo);

private:
    std::unique_ptr<MySqlPool> _pool;
};