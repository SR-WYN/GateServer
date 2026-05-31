// VerifyGrpcClient.h - 通过 gRPC 向 VerifyServer 请求验证码
#pragma once

#include "Singleton.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class RPConPool;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;

public:
    // 请求向指定 email 发送验证码，返回 gRPC 响应
    GetVerifyRsp getVerifyCode(std::string email);
    ~VerifyGrpcClient() override;

private:
    VerifyGrpcClient();
    std::unique_ptr<RPConPool> _pool;
};
