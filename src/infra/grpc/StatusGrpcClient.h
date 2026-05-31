// StatusGrpcClient.h - 通过 gRPC 向 StatusServer 查询可用 ChatServer
#pragma once

#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include "Singleton.h"

using grpc::ClientContext;
using grpc::Status;

using message::GetChatServerRsp;
using message::GetChatServerReq;
using message::StatusService;

class StatusConPool;

class StatusGrpcClient : public Singleton<StatusGrpcClient>
{
    friend class Singleton<StatusGrpcClient>;
public:
    ~StatusGrpcClient() override;
    // 根据 uid 获取分配的 ChatServer 地址（host:port）和 token
    GetChatServerRsp getChatServer(int uid);
private:
    StatusGrpcClient();
    std::unique_ptr<StatusConPool> _pool;
};
