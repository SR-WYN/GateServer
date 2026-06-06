// UserCacheAdapter.cpp - IUserCache 适配器实现
#include "UserCacheAdapter.h"
#include "RedisMgr.h"

bool UserCacheAdapter::setUserEmail(const std::string& name,
                                    const std::string& email)
{
    return RedisMgr::getInstance().set(name, email);
}

bool UserCacheAdapter::existsKey(const std::string& key)
{
    return RedisMgr::getInstance().existsKey(key);
}
