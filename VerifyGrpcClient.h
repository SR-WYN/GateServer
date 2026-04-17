#pragma once

#include "Singleton.h"
#include "const.h"
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
    GetVerifyRsp GetVerifyCode(std::string email);
    ~VerifyGrpcClient() override;

private:
    VerifyGrpcClient();
    std::unique_ptr<RPConPool> _pool;
};