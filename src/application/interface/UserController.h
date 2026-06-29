// UserController.h - HTTP 用户控制器接口
// 薄层：解析 HTTP 请求，调用 UserService，构造 JSON 响应
#pragma once

#include <memory>

class HttpConnection;

class UserController
{
public:
    virtual ~UserController() = default;

    virtual void handleRegister(std::shared_ptr<HttpConnection> conn) = 0;
    virtual void handleLogin(std::shared_ptr<HttpConnection> conn) = 0;
    virtual void handleResetPwd(std::shared_ptr<HttpConnection> conn) = 0;
    virtual void handleLogout(std::shared_ptr<HttpConnection> conn) = 0;
};
