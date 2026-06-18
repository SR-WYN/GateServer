// UserDaoImpl.cpp - UserDao 适配器实现
#include "UserDaoImpl.h"
#include "MysqlMgr.h"

int UserDaoImpl::regUser(const std::string &name, const std::string &email, const std::string &pwd,
                         const std::string &nick, int sex)
{
    return MysqlMgr::getInstance().dao().regUser(name, email, pwd, nick, sex);
}

bool UserDaoImpl::userNameExists(const std::string &name)
{
    return MysqlMgr::getInstance().dao().userNameExists(name);
}

bool UserDaoImpl::emailExists(const std::string &email)
{
    return MysqlMgr::getInstance().dao().emailExists(email);
}

bool UserDaoImpl::checkEmail(const std::string &email, const std::string &name)
{
    return MysqlMgr::getInstance().dao().checkEmail(email, name);
}

bool UserDaoImpl::updatePwd(const std::string &email, const std::string &pwd)
{
    return MysqlMgr::getInstance().dao().updatePwd(email, pwd);
}

bool UserDaoImpl::checkPwd(const std::string &email, const std::string &pwd, UserInfo &userInfo)
{
    return MysqlMgr::getInstance().dao().checkPwd(email, pwd, userInfo);
}
