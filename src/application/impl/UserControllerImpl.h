// UserControllerImpl.h - HTTP 用户控制器实现
#pragma once

#include "UserController.h"
#include <memory>

class UserService;

class UserControllerImpl final : public UserController
{
public:
    explicit UserControllerImpl(std::shared_ptr<UserService> userService);

    void handleRegister(std::shared_ptr<HttpConnection> conn) override;
    void handleLogin(std::shared_ptr<HttpConnection> conn) override;
    void handleResetPwd(std::shared_ptr<HttpConnection> conn) override;

private:
    std::shared_ptr<UserService> _userService;
};
