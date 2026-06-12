// UserCacheImpl.h - UserCache 适配器
// 将 RedisMgr 封装为 UserCache 接口
#pragma once

#include "UserCache.h"

/// UserCache 的 Redis 实现适配器
class UserCacheImpl : public UserCache
{
public:
    bool setUserEmail(const std::string &name, const std::string &email) override;
    bool existsKey(const std::string &key) override;
};
