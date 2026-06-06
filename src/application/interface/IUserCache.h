// IUserCache.h - 用户缓存操作接口
// 抽象用户信息的缓存操作，通常基于 Redis 实现
#pragma once

#include <string>

/// 用户缓存接口
class IUserCache {
public:
    virtual ~IUserCache() = default;

    /// 缓存用户名到邮箱的映射
    virtual bool setUserEmail(const std::string& name,
                              const std::string& email) = 0;

    /// 检查 key 是否存在
    virtual bool existsKey(const std::string& key) = 0;
};
