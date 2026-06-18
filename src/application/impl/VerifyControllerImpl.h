// VerifyControllerImpl.h - HTTP 验证码控制器实现
#pragma once

#include "VerifyController.h"
#include <memory>

class VerifyService;

class VerifyControllerImpl final : public VerifyController
{
public:
    explicit VerifyControllerImpl(std::shared_ptr<VerifyService> verifyService);

    void handleGetVerifyCode(std::shared_ptr<HttpConnection> conn) override;

private:
    std::shared_ptr<VerifyService> _verifyService;
};
