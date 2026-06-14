// UserController.h - 用户相关业务：注册、登录、重置密码
// 通过依赖注入获取基础设施服务，不直接依赖具体实现
#pragma once

#include <memory>

class HttpConnection;
class UserDao;
class VerifyCodeCache;
class UserCache;
class StatusRpcClient;

/// 用户业务控制器
class UserController
{
public:
    /// 构造函数注入所有依赖
    UserController(std::shared_ptr<UserDao> userDao, std::shared_ptr<VerifyCodeCache> verifyCache,
                   std::shared_ptr<UserCache> userCache,
                   std::shared_ptr<StatusRpcClient> statusRpc);

    void handleRegister(std::shared_ptr<HttpConnection> conn);
    void handleLogin(std::shared_ptr<HttpConnection> conn);
    void handleResetPwd(std::shared_ptr<HttpConnection> conn);

    /// 处理用户下线通知
    /// @param uid 用户 ID
    void handleUserOffline(int uid);

private:
    std::shared_ptr<UserDao> _userDao;
    std::shared_ptr<VerifyCodeCache> _verifyCache;
    std::shared_ptr<UserCache> _userCache;
    std::shared_ptr<StatusRpcClient> _statusRpc;

    // 配置常量
    static constexpr int SESSION_TTL_SECONDS = 86400;      // 会话缓存 24 小时
    static constexpr int TOKEN_VALIDITY_SECONDS = 7200;    // Token 有效期 2 小时

    /// 获取当前时间戳（秒）
    static int64_t getCurrentTimestamp();
};
