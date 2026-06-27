// UserCacheImpl.cpp - UserCache 适配器实现
#include "UserCacheImpl.h"

#include "Log.h"
#include "RedisMgr.h"

#include <chrono>

// ========== 原有方法 ==========

bool UserCacheImpl::setUserEmail(const std::string &name, const std::string &email)
{
    bool ok = RedisMgr::getInstance().set(name, email);
    if (ok)
    {
        LOGI(LogModule::Redis, "User email cached name={} email={}", name, email);
    }
    else
    {
        LOGE(LogModule::Redis, "User email cache failed name={} email={}", name, email);
    }
    return ok;
}

bool UserCacheImpl::existsKey(const std::string &key)
{
    bool ok = RedisMgr::getInstance().existsKey(key);
    LOGI(LogModule::Redis, "existsKey key={} result={}", key, ok);
    return ok;
}

// ========== 新增会话缓存方法 ==========

bool UserCacheImpl::saveSession(const UserSession &session, int ttl_seconds)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string session_key = sessionKey(session._uid);

        // 使用 Hash 存储会话信息
        redis.hSet(session_key, "token", session._token);
        redis.hSet(session_key, "e_mail", session._email);
        redis.hSet(session_key, "user_name", session._user_name);
        redis.hSet(session_key, "chat_host", session._chat_host);
        redis.hSet(session_key, "chat_port", session._chat_port);
        redis.hSet(session_key, "login_time", std::to_string(session._login_time));
        redis.hSet(session_key, "expire_at", std::to_string(session._expire_at));

        // 设置过期时间
        redis.expire(session_key, ttl_seconds);

        // 建立索引
        redis.set(nameKey(session._user_name), std::to_string(session._uid), ttl_seconds);
        redis.set(emailKey(session._email), std::to_string(session._uid), ttl_seconds);
        redis.set(tokenKey(session._token), std::to_string(session._uid), ttl_seconds);

        // 加入在线集合
        redis.sAdd(onlineUsersKey(), std::to_string(session._uid));

        LOGI(LogModule::Redis, "Session saved for uid={} email={} host={}:{} expire_at={}",
             session._uid, session._email, session._chat_host, session._chat_port,
             session._expire_at);
        return true;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to save session for uid={}: {}", session._uid, e.what());
        return false;
    }
}

std::optional<UserSession> UserCacheImpl::getSession(int uid)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string session_key = sessionKey(uid);

        std::map<std::string, std::string> fields;
        if (!redis.hGetAll(session_key, fields) || fields.empty())
        {
            return std::nullopt;
        }

        UserSession session;
        session._uid = uid;
        session._token = fields["token"];
        session._email = fields["e_mail"];
        session._user_name = fields["user_name"];
        session._chat_host = fields["chat_host"];
        session._chat_port = fields["chat_port"];
        session._login_time = std::stoll(fields["login_time"]);
        session._expire_at = std::stoll(fields["expire_at"]);

        LOGI(LogModule::Redis, "Session get hit for uid={} email={} expire_at={}", session._uid,
             session._email, session._expire_at);
        return session;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to get session for uid={}: {}", uid, e.what());
        return std::nullopt;
    }
}

std::optional<int> UserCacheImpl::getUidByToken(const std::string &token)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string uid_str;
        if (redis.get(tokenKey(token), uid_str) && !uid_str.empty())
        {
            return std::stoi(uid_str);
        }
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to get uid by token: {}", e.what());
        return std::nullopt;
    }
}

std::optional<int> UserCacheImpl::getUidByName(const std::string &user_name)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string uid_str;
        if (redis.get(nameKey(user_name), uid_str) && !uid_str.empty())
        {
            return std::stoi(uid_str);
        }
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to get uid by name: {}", e.what());
        return std::nullopt;
    }
}

std::optional<int> UserCacheImpl::getUidByEmail(const std::string &email)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string uid_str;
        if (redis.get(emailKey(email), uid_str) && !uid_str.empty())
        {
            return std::stoi(uid_str);
        }
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to get uid by email: {}", e.what());
        return std::nullopt;
    }
}

bool UserCacheImpl::removeSession(int uid)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string session_key = sessionKey(uid);

        // 先获取 Token 和用户名用于清理索引
        std::string token = redis.hGet(session_key, "token");
        std::string user_name = redis.hGet(session_key, "user_name");
        std::string email = redis.hGet(session_key, "e_mail");

        // 删除会话数据
        redis.del(session_key);

        // 清理索引
        if (!user_name.empty())
        {
            redis.del(nameKey(user_name));
        }
        if (!email.empty())
        {
            redis.del(emailKey(email));
        }
        if (!token.empty())
        {
            redis.del(tokenKey(token));
        }

        // 从在线集合移除
        redis.sRem(onlineUsersKey(), std::to_string(uid));

        LOGI(LogModule::Redis, "Session removed for uid={}", uid);
        return true;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to remove session for uid={}: {}", uid, e.what());
        return false;
    }
}

bool UserCacheImpl::extendSession(int uid, int ttl_seconds)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string session_key = sessionKey(uid);

        // 延长会话过期时间
        redis.expire(session_key, ttl_seconds);

        // 延长索引过期时间
        std::string user_name = redis.hGet(session_key, "user_name");
        if (!user_name.empty())
        {
            redis.expire(nameKey(user_name), ttl_seconds);
        }

        std::string email = redis.hGet(session_key, "e_mail");
        if (!email.empty())
        {
            redis.expire(emailKey(email), ttl_seconds);
        }

        std::string token = redis.hGet(session_key, "token");
        if (!token.empty())
        {
            redis.expire(tokenKey(token), ttl_seconds);
        }

        LOGI(LogModule::Redis, "Session extended for uid={} ttl={}s", uid, ttl_seconds);
        return true;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to extend session for uid={}: {}", uid, e.what());
        return false;
    }
}

bool UserCacheImpl::isOnline(int uid)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        return redis.sIsMember(onlineUsersKey(), std::to_string(uid));
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to check online status for uid={}: {}", uid, e.what());
        return false;
    }
}

// ========== 新增用户凭证缓存方法 ==========

bool UserCacheImpl::cacheUserCredential(const std::string &email, int uid, const std::string &name,
                                        const std::string &pwd_hash, int ttl_seconds)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string key = userCredKey(email);

        redis.hSet(key, "uid", std::to_string(uid));
        redis.hSet(key, "name", name);
        redis.hSet(key, "pwd_hash", pwd_hash);
        redis.expire(key, ttl_seconds);

        LOGI(LogModule::Redis, "User credential cached for {} uid={} ttl={}s", email, uid,
             ttl_seconds);
        return true;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to cache user credential for {}: {}", email, e.what());
        return false;
    }
}

bool UserCacheImpl::getUserCredential(const std::string &email, int &uid, std::string &name,
                                      std::string &pwd_hash)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        std::string key = userCredKey(email);

        std::map<std::string, std::string> fields;
        if (!redis.hGetAll(key, fields) || fields.empty())
        {
            return false;
        }

        uid = std::stoi(fields["uid"]);
        name = fields["name"];
        pwd_hash = fields["pwd_hash"];

        LOGI(LogModule::Redis, "User credential hit for {} uid={}", email, uid);
        return !name.empty() && !pwd_hash.empty();
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to get user credential for {}: {}", email, e.what());
        return false;
    }
}

bool UserCacheImpl::invalidateUserCredential(const std::string &email)
{
    try
    {
        auto &redis = RedisMgr::getInstance();
        redis.del(userCredKey(email));
        LOGI(LogModule::Redis, "User credential invalidated for {}", email);
        return true;
    }
    catch (const std::exception &e)
    {
        LOGE(LogModule::Redis, "Failed to invalidate user credential for {}: {}", email, e.what());
        return false;
    }
}

// ========== 私有辅助方法 ==========

std::string UserCacheImpl::sessionKey(int uid)
{
    return std::string(SESSION_PREFIX) + std::to_string(uid);
}

std::string UserCacheImpl::nameKey(const std::string &name)
{
    return std::string(NAME_PREFIX) + name;
}

std::string UserCacheImpl::emailKey(const std::string &email)
{
    return std::string(EMAIL_PREFIX) + email;
}

std::string UserCacheImpl::tokenKey(const std::string &token)
{
    return std::string(TOKEN_PREFIX) + token;
}

std::string UserCacheImpl::onlineUsersKey()
{
    return ONLINE_USERS_KEY;
}

std::string UserCacheImpl::userCredKey(const std::string &email)
{
    return std::string(USER_CRED_PREFIX) + email;
}
