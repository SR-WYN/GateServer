// UserCacheImpl.h - UserCache 适配器
// 将 RedisMgr 封装为 UserCache 接口
#pragma once

#include "UserCache.h"
#include "redis_keys.h"

#include <map>

/// UserCache 的 Redis 实现适配器
class UserCacheImpl : public UserCache
{
public:
    // ========== 原有方法 ==========
    bool setUserEmail(const std::string &name, const std::string &email) override;
    bool existsKey(const std::string &key) override;

    // ========== 新增会话缓存方法 ==========
    bool saveSession(const UserSession &session, int ttl_seconds) override;
    std::optional<UserSession> getSession(int uid) override;
    bool removeSession(int uid) override;
    bool extendSession(int uid, int ttl_seconds) override;
    bool isOnline(int uid) override;

    // ========== 新增用户凭证缓存方法 ==========
    bool cacheUserCredential(const std::string &email, int uid, const std::string &name,
                             const std::string &pwd_hash, int ttl_seconds) override;
    bool getUserCredential(const std::string &email, int &uid, std::string &name,
                           std::string &pwd_hash) override;
    bool invalidateUserCredential(const std::string &email) override;

private:
    static std::string sessionKey(int uid);
    static std::string userCredKey(const std::string &email);
};
