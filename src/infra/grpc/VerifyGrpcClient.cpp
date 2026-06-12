// VerifyGrpcClient.cpp - 向 VerifyServer 请求验证码的 gRPC 客户端实现
#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "RPConPool.h"
#include "error_codes.h"
#include "message.grpc.pb.h"
#include "message.pb.h"

#include <memory>

VerifyGrpcClient::VerifyGrpcClient()
{
    auto &gCfgMgr = ConfigMgr::getInstance();
    std::string host = gCfgMgr["VerifyServer"]["Host"];
    std::string port = gCfgMgr["VerifyServer"]["Port"];
    Log::info(LogModule::Grpc, "VerifyGrpcClient connecting to VerifyServer at {}:{}", host, port);
    _pool.reset(new RPConPool(5, host, port));
}

VerifyGrpcClient::~VerifyGrpcClient()
{
    Log::info(LogModule::Grpc, "VerifyGrpcClient destroyed");
}

GetVerifyRsp VerifyGrpcClient::getVerifyCode(std::string email)
{
    Log::info(LogModule::Grpc, "VerifyGrpcClient::getVerifyCode email={}", email);
    ClientContext context;
    GetVerifyRsp reply;
    GetVerifyReq request;
    request.set_email(email);
    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "getVerifyCode: failed to get connection from pool");
        reply.set_error(ErrorCodes::RPC_FAILED);
        return reply;
    }
    Status status = stub->GetVerifyCode(&context, request, &reply);
    _pool->returnConnection(std::move(stub));
    if (status.ok())
    {
        Log::info(LogModule::Grpc, "getVerifyCode success: email={}, error={}", email,
                  reply.error());
        return reply;
    }
    else
    {
        Log::error(LogModule::Grpc, "getVerifyCode RPC failed: email={}, error_code={}, msg={}",
                   email, status.error_code(), status.error_message());
        reply.set_error(ErrorCodes::RPC_FAILED);
        return reply;
    }
}
