// VerifyController.h - 验证码相关业务
#pragma once

#include <memory>

class HttpConnection;

class VerifyController
{
public:
    static void handleGetVerifyCode(std::shared_ptr<HttpConnection> conn);
};
