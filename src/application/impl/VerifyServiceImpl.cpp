// VerifyServiceImpl.cpp - 验证码业务服务实现
#include "VerifyServiceImpl.h"

#include "Log.h"
#include "LogModule.h"
#include "VerifyRpcClient.h"
#include "error_codes.h"

#include <chrono>

VerifyServiceImpl::VerifyServiceImpl(std::shared_ptr<VerifyRpcClient> verifyRpc)
    : _verifyRpc(std::move(verifyRpc))
{
}

message::GetVerifyRsp VerifyServiceImpl::getVerifyCode(const std::string &email)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "VerifyServiceImpl::getVerifyCode | email={}", email);

    auto rsp = _verifyRpc->getVerifyCode(email);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (rsp.error() == ErrorCodes::SUCCESS)
    {
        LOGI(LogModule::Http, "VerifyServiceImpl::getVerifyCode success email={} cost={}ms",
             email, cost_ms);
    }
    else
    {
        LOGW(LogModule::Http, "VerifyServiceImpl::getVerifyCode failed email={} err={} cost={}ms",
             email, rsp.error(), cost_ms);
    }

    return rsp;
}
