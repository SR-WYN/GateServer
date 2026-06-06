// VerifyRpcClientAdapter.h - IVerifyRpcClient 适配器
// 将 VerifyGrpcClient 封装为 IVerifyRpcClient 接口
#pragma once

#include "IVerifyRpcClient.h"

/// IVerifyRpcClient 的 gRPC 实现适配器
class VerifyRpcClientAdapter : public IVerifyRpcClient {
public:
    GetVerifyRsp getVerifyCode(const std::string& email) override;
};
