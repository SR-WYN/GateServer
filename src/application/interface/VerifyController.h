// VerifyController.h - HTTP 验证码控制器接口
#pragma once

#include <memory>

class HttpConnection;

class VerifyController
{
public:
    virtual ~VerifyController() = default;

    virtual void handleGetVerifyCode(std::shared_ptr<HttpConnection> conn) = 0;
};
