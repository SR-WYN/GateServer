// StatusRpcClientImpl.h - StatusRpcClient 适配器
// 将 StatusGrpcClient 封装为 StatusRpcClient 接口
#pragma once

#include "StatusRpcClient.h"

/// StatusRpcClient 的 gRPC 实现适配器
class StatusRpcClientImpl : public StatusRpcClient
{
public:
    GetChatServerRsp getChatServer(int uid) override;
    int validateToken(int uid, const std::string& token) override;
};
