// UserController.h - 用户相关业务：注册、登录、重置密码
// 通过依赖注入获取基础设施服务，不直接依赖具体实现
#pragma once

#include <memory>

class HttpConnection;
class IUserDao;
class IVerifyCodeCache;
class IUserCache;
class IStatusRpcClient;

class UserController {
public:
    /// 构造函数注入所有依赖
    UserController(
        std::shared_ptr<IUserDao> userDao,
        std::shared_ptr<IVerifyCodeCache> verifyCache,
        std::shared_ptr<IUserCache> userCache,
        std::shared_ptr<IStatusRpcClient> statusRpc);

    void handleRegister(std::shared_ptr<HttpConnection> conn);
    void handleLogin(std::shared_ptr<HttpConnection> conn);
    void handleResetPwd(std::shared_ptr<HttpConnection> conn);

private:
    std::shared_ptr<IUserDao> _userDao;
    std::shared_ptr<IVerifyCodeCache> _verifyCache;
    std::shared_ptr<IUserCache> _userCache;
    std::shared_ptr<IStatusRpcClient> _statusRpc;
};
