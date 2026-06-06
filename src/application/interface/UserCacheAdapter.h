// UserCacheAdapter.h - IUserCache 适配器
// 将 RedisMgr 封装为 IUserCache 接口
#pragma once

#include "IUserCache.h"

/// IUserCache 的 Redis 实现适配器
class UserCacheAdapter : public IUserCache {
public:
    bool setUserEmail(const std::string& name,
                      const std::string& email) override;
    bool existsKey(const std::string& key) override;
};
