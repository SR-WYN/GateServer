// VerifyRpcClient.h - VerifyServer gRPC 通信接口
// 抽象与 VerifyServer 的 RPC 调用，用于获取验证码
#pragma once

#include "message.pb.h"
#include <string>

using message::GetVerifyRsp;

/// VerifyServer gRPC 客户端接口
class VerifyRpcClient
{
public:
    virtual ~VerifyRpcClient() = default;

    /// 请求向指定邮箱发送验证码
    virtual GetVerifyRsp getVerifyCode(const std::string &email) = 0;
};
