// UserCacheImpl.cpp - UserCache 适配器实现
#include "UserCacheImpl.h"
#include "RedisMgr.h"

bool UserCacheImpl::setUserEmail(const std::string &name, const std::string &email)
{
    return RedisMgr::getInstance().set(name, email);
}

bool UserCacheImpl::existsKey(const std::string &key)
{
    return RedisMgr::getInstance().existsKey(key);
}
