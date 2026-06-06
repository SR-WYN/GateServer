// VerifyRpcClientAdapter.cpp - IVerifyRpcClient 适配器实现
#include "VerifyRpcClientAdapter.h"
#include "VerifyGrpcClient.h"

GetVerifyRsp VerifyRpcClientAdapter::getVerifyCode(const std::string& email)
{
    return VerifyGrpcClient::getInstance().getVerifyCode(email);
}
