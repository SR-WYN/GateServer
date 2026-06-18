// VerifyServiceImpl.h - 验证码业务服务实现
#pragma once

#include "VerifyService.h"
#include <memory>

class VerifyRpcClient;

class VerifyServiceImpl final : public VerifyService
{
public:
    explicit VerifyServiceImpl(std::shared_ptr<VerifyRpcClient> verifyRpc);

    message::GetVerifyRsp getVerifyCode(const std::string &email) override;

private:
    std::shared_ptr<VerifyRpcClient> _verifyRpc;
};
