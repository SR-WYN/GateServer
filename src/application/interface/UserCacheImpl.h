// UserCacheImpl.h - UserCache 适配器
// 将 RedisMgr 封装为 UserCache 接口
#pragma once

#include "UserCache.h"

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
    std::optional<int> getUidByToken(const std::string &token) override;
    std::optional<int> getUidByName(const std::string &user_name) override;
    std::optional<int> getUidByEmail(const std::string &email) override;
    bool removeSession(int uid) override;
    bool extendSession(int uid, int ttl_seconds) override;
    bool isOnline(int uid) override;

private:
    static std::string sessionKey(int uid);
    static std::string nameKey(const std::string &name);
    static std::string emailKey(const std::string &email);
    static std::string tokenKey(const std::string &token);
    static std::string onlineUsersKey();

    static constexpr char SESSION_PREFIX[] = "gate:session:";
    static constexpr char NAME_PREFIX[] = "gate:name:";
    static constexpr char EMAIL_PREFIX[] = "gate:email:";
    static constexpr char TOKEN_PREFIX[] = "gate:token:";
    static constexpr char ONLINE_USERS_KEY[] = "gate:online_users";
};
