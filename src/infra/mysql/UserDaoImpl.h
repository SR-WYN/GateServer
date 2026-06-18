// UserDaoImpl.h - UserDao 适配器
// 将 MysqlMgr 封装为 UserDao 接口
#pragma once

#include "UserDao.h"

/// UserDao 的 MySQL 实现适配器
/// 内部委托给 MysqlMgr::getInstance().dao()
class UserDaoImpl : public UserDao
{
public:
    int regUser(const std::string &name, const std::string &email, const std::string &pwd,
                const std::string &nick = "", int sex = 0) override;

    bool userNameExists(const std::string &name) override;
    bool emailExists(const std::string &email) override;
    bool checkEmail(const std::string &email, const std::string &name) override;
    bool updatePwd(const std::string &email, const std::string &pwd) override;
    bool checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo) override;
};
