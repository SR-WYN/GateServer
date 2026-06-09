// UserDaoAdapter.cpp - IUserDao 适配器实现
#include "UserDaoAdapter.h"
#include "MysqlMgr.h"

int UserDaoAdapter::regUser(const std::string& name,
                            const std::string& email,
                            const std::string& pwd,
                            const std::string& nick,
                            int sex)
{
    return MysqlMgr::getInstance().dao().regUser(name, email, pwd, nick, sex);
}

bool UserDaoAdapter::userNameExists(const std::string& name)
{
    return MysqlMgr::getInstance().dao().userNameExists(name);
}

bool UserDaoAdapter::emailExists(const std::string& email)
{
    return MysqlMgr::getInstance().dao().emailExists(email);
}

bool UserDaoAdapter::checkEmail(const std::string& email,
                                const std::string& name)
{
    return MysqlMgr::getInstance().dao().checkEmail(email, name);
}

bool UserDaoAdapter::updatePwd(const std::string& email,
                               const std::string& pwd)
{
    return MysqlMgr::getInstance().dao().updatePwd(email, pwd);
}

bool UserDaoAdapter::checkPwd(const std::string& email,
                              const std::string& pwd,
                              UserInfo& userInfo)
{
    return MysqlMgr::getInstance().dao().checkPwd(email, pwd, userInfo);
}
