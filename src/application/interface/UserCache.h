// UserCache.h - 用户缓存操作接口
// 抽象用户信息的缓存操作，通常基于 Redis 实现
#pragma once

#include <cstdint>
#include <optional>
#include <string>

/// 用户会话信息结构体
struct UserSession {
    int _uid = 0;
    std::string _token;
    std::string _email;
    std::string _user_name;
    std::string _chat_host;
    std::string _chat_port;
    int64_t _login_time = 0;
    int64_t _expire_at = 0;
};

/// 用户缓存接口
class UserCache
{
public:
    virtual ~UserCache() = default;

    // ========== 原有方法 ==========

    /// 缓存用户名到邮箱的映射
    virtual bool setUserEmail(const std::string &name, const std::string &email) = 0;

    /// 检查 key 是否存在
    virtual bool existsKey(const std::string &key) = 0;

    // ========== 新增会话缓存方法 ==========

    /// 保存用户会话信息
    /// @param session 会话信息
    /// @param ttl_seconds 过期时间（秒）
    /// @return 是否成功
    virtual bool saveSession(const UserSession &session, int ttl_seconds) = 0;

    /// 根据 UID 获取会话信息
    /// @param uid 用户 ID
    /// @return 会话信息（不存在返回 nullopt）
    virtual std::optional<UserSession> getSession(int uid) = 0;

    /// 根据 Token 获取 UID
    /// @param token 登录令牌
    /// @return 用户 ID（不存在返回 nullopt）
    virtual std::optional<int> getUidByToken(const std::string &token) = 0;

    /// 根据用户名获取 UID
    /// @param user_name 用户名
    /// @return 用户 ID（不存在返回 nullopt）
    virtual std::optional<int> getUidByName(const std::string &user_name) = 0;

    /// 根据邮箱获取 UID
    /// @param email 邮箱
    /// @return 用户 ID（不存在返回 nullopt）
    virtual std::optional<int> getUidByEmail(const std::string &email) = 0;

    /// 删除会话信息
    /// @param uid 用户 ID
    /// @return 是否成功
    virtual bool removeSession(int uid) = 0;

    /// 延长会话过期时间
    /// @param uid 用户 ID
    /// @param ttl_seconds 新的过期时间（秒）
    /// @return 是否成功
    virtual bool extendSession(int uid, int ttl_seconds) = 0;

    /// 检查用户是否在线
    /// @param uid 用户 ID
    /// @return 是否在线
    virtual bool isOnline(int uid) = 0;
};
