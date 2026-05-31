// UserController.h - 用户相关业务：注册、登录、重置密码
#pragma once

#include <memory>

class HttpConnection;

class UserController
{
public:
    static void handleRegister(std::shared_ptr<HttpConnection> conn);
    static void handleLogin(std::shared_ptr<HttpConnection> conn);
    static void handleResetPwd(std::shared_ptr<HttpConnection> conn);
};
