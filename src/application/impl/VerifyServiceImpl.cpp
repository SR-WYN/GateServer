// VerifyServiceImpl.cpp - 验证码业务服务实现
#include "VerifyServiceImpl.h"

#include "Log.h"
#include "LogModule.h"
#include "VerifyRpcClient.h"

VerifyServiceImpl::VerifyServiceImpl(std::shared_ptr<VerifyRpcClient> verifyRpc)
    : _verifyRpc(std::move(verifyRpc))
{
}

message::GetVerifyRsp VerifyServiceImpl::getVerifyCode(const std::string &email)
{
    LOGI(LogModule::Http, "VerifyServiceImpl::getVerifyCode | email={}", email);
    return _verifyRpc->getVerifyCode(email);
}
