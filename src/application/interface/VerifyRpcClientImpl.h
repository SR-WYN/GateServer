// VerifyRpcClientImpl.h - VerifyRpcClient 适配器
// 将 VerifyGrpcClient 封装为 VerifyRpcClient 接口
#pragma once

#include "VerifyRpcClient.h"

/// VerifyRpcClient 的 gRPC 实现适配器
class VerifyRpcClientImpl : public VerifyRpcClient
{
public:
    GetVerifyRsp getVerifyCode(const std::string &email) override;
};
