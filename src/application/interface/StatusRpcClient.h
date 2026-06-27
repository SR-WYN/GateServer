// StatusRpcClient.h - StatusServer gRPC 通信接口
// 抽象与 StatusServer 的 RPC 调用，用于获取 ChatServer 分配
#pragma once

#include "message.pb.h"

using message::GetChatServerRsp;

/// StatusServer gRPC 客户端接口
class StatusRpcClient
{
public:
    virtual ~StatusRpcClient() = default;

    /// 根据 uid 获取分配的 ChatServer 地址和 token
    virtual GetChatServerRsp getChatServer(int uid) = 0;

    /// 校验用户 token 是否有效
    /// @return ErrorCodes::SUCCESS 表示有效，其他表示无效或 RPC 失败
    virtual int validateToken(int uid, const std::string& token) = 0;
};
