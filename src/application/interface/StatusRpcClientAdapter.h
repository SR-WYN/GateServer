// StatusRpcClientAdapter.h - IStatusRpcClient 适配器
// 将 StatusGrpcClient 封装为 IStatusRpcClient 接口
#pragma once

#include "IStatusRpcClient.h"

/// IStatusRpcClient 的 gRPC 实现适配器
class StatusRpcClientAdapter : public IStatusRpcClient {
public:
    GetChatServerRsp getChatServer(int uid) override;
};
