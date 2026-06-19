// UserDaoImpl.h - UserDao 的 MySQL 实现
#pragma once

#include "UserDao.h"
#include <memory>

class MySqlMgr;

class UserDaoImpl : public UserDao
{
public:
    explicit UserDaoImpl(std::shared_ptr<MySqlMgr> mysql_mgr);

    int regUser(const std::string &name, const std::string &email, const std::string &pwd,
                const std::string &nick = "", int sex = 0) override;

    bool userNameExists(const std::string &name) override;
    bool emailExists(const std::string &email) override;
    bool checkEmail(const std::string &email, const std::string &name) override;
    bool updatePwd(const std::string &email, const std::string &pwd) override;
    bool checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo) override;

private:
    std::shared_ptr<MySqlMgr> _mysql_mgr;
};
