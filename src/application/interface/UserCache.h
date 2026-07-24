// UserCache.h - 用户缓存操作接口
// 抽象用户信息的缓存操作，通常基于 Redis 实现
#pragma once

#include <cstdint>
#include <optional>
#include <string>

/// 用户会话信息结构体
struct UserSession
{
    int _uid = 0;
    std::string _token;
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

    // ========== 新增用户凭证缓存方法 ==========

    /// 缓存用户凭证信息
    /// @param email 用户邮箱（作为缓存键）
    /// @param uid 用户 ID
    /// @param name 用户名
    /// @param pwd_hash 密码哈希值（与 MySQL 中 pwd 字段一致）
    /// @param ttl_seconds 过期时间（秒）
    /// @return 是否成功
    virtual bool cacheUserCredential(const std::string &email, int uid, const std::string &name,
                                     const std::string &pwd_hash, int ttl_seconds) = 0;

    /// 获取缓存的用户凭证
    /// @param email 用户邮箱
    /// @param uid 输出参数，返回用户 ID
    /// @param name 输出参数，返回用户名
    /// @param pwd_hash 输出参数，返回密码哈希
    /// @return 是否存在且字段完整
    virtual bool getUserCredential(const std::string &email, int &uid, std::string &name,
                                   std::string &pwd_hash) = 0;

    /// 使用户凭证缓存失效（密码重置/修改时调用）
    /// @param email 用户邮箱
    /// @return 是否成功
    virtual bool invalidateUserCredential(const std::string &email) = 0;
};
