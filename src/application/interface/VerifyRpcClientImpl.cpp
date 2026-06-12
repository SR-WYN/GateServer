// VerifyRpcClientImpl.cpp - VerifyRpcClient 适配器实现
#include "VerifyRpcClientImpl.h"
#include "VerifyGrpcClient.h"

GetVerifyRsp VerifyRpcClientImpl::getVerifyCode(const std::string &email)
{
    return VerifyGrpcClient::getInstance().getVerifyCode(email);
}
