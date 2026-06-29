// StatusGrpcClient.cpp - 向 StatusServer 查询可用 ChatServer 的 gRPC 客户端实现
#include "StatusGrpcClient.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "StatusConPool.h"
#include "error_codes.h"
#include "defer.h"

#include <chrono>

StatusGrpcClient::StatusGrpcClient()
{
    auto &gCfgMgr = ConfigMgr::getInstance();
    std::string host = gCfgMgr["StatusServer"]["Host"];
    std::string port = gCfgMgr["StatusServer"]["Port"];
    Log::info(LogModule::Grpc, "StatusGrpcClient connecting to StatusServer at {}:{}", host, port);
    _pool.reset(new StatusConPool(5, host, port));
}

StatusGrpcClient::~StatusGrpcClient()
{
    Log::info(LogModule::Grpc, "StatusGrpcClient destroyed");
}

GetChatServerRsp StatusGrpcClient::getChatServer(int uid)
{
    const auto start = std::chrono::steady_clock::now();
    Log::info(LogModule::Grpc, "StatusGrpcClient::getChatServer uid={}", uid);

    ClientContext context;
    GetChatServerRsp reply;
    GetChatServerReq request;
    request.set_uid(uid);
    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "getChatServer: failed to get connection from pool");
        reply.set_error(ErrorCodes::RPC_FAILED);
        return reply;
    }
    Status status = stub->GetChatServer(&context, request, &reply);
    utils::Defer defer([&stub, this]() {
        _pool->returnConnection(std::move(stub));
    });

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    if (status.ok())
    {
        if (reply.error() == ErrorCodes::SUCCESS)
        {
            Log::info(LogModule::Grpc,
                      "getChatServer success: uid={}, host={}, port={} cost={}ms", uid,
                      reply.host(), reply.port(), cost_ms);
        }
        else
        {
            Log::warn(LogModule::Grpc, "getChatServer failed: uid={}, err={} cost={}ms", uid,
                      reply.error(), cost_ms);
        }
        return reply;
    }
    else
    {
        Log::error(LogModule::Grpc, "getChatServer RPC failed: uid={}, error_code={}, msg={}",
                   uid, status.error_code(), status.error_message());
        reply.set_error(ErrorCodes::RPC_FAILED);
        return reply;
    }
}

int StatusGrpcClient::validateToken(int uid, const std::string& token)
{
    const auto start = std::chrono::steady_clock::now();
    Log::info(LogModule::Grpc, "StatusGrpcClient::validateToken uid={} token={}", uid, token);

    ClientContext context;
    message::ValidateTokenReq request;
    message::ValidateTokenRsp reply;
    request.set_uid(uid);
    request.set_token(token);

    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "validateToken: failed to get connection from pool");
        return ErrorCodes::RPC_FAILED;
    }

    Status status = stub->ValidateToken(&context, request, &reply);
    utils::Defer defer([&stub, this]() {
        _pool->returnConnection(std::move(stub));
    });

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    if (!status.ok())
    {
        Log::error(LogModule::Grpc, "validateToken RPC failed: uid={}, error_code={}, msg={}",
                   uid, status.error_code(), status.error_message());
        return ErrorCodes::RPC_FAILED;
    }

    if (reply.error() == ErrorCodes::SUCCESS)
    {
        Log::info(LogModule::Grpc, "validateToken success: uid={} cost={}ms", uid, cost_ms);
    }
    else
    {
        Log::warn(LogModule::Grpc, "validateToken failed: uid={} err={} cost={}ms", uid,
                  reply.error(), cost_ms);
    }

    return reply.error();
}

bool StatusGrpcClient::logout(int uid)
{
    const auto start = std::chrono::steady_clock::now();
    Log::info(LogModule::Grpc, "StatusGrpcClient::logout uid={}", uid);

    ClientContext context;
    message::LogoutReq request;
    message::LogoutRsp reply;
    request.set_uid(uid);

    auto stub = _pool->getConnection();
    if (!stub)
    {
        Log::error(LogModule::Grpc, "logout: failed to get connection from pool");
        return false;
    }

    Status status = stub->Logout(&context, request, &reply);
    utils::Defer defer([&stub, this]() {
        _pool->returnConnection(std::move(stub));
    });

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    if (!status.ok())
    {
        Log::error(LogModule::Grpc, "logout RPC failed: uid={}, error_code={}, msg={}",
                   uid, status.error_code(), status.error_message());
        return false;
    }

    if (reply.error() != ErrorCodes::SUCCESS)
    {
        Log::warn(LogModule::Grpc, "logout failed: uid={}, err={} cost={}ms", uid,
                  reply.error(), cost_ms);
        return false;
    }

    Log::info(LogModule::Grpc, "logout success: uid={} cost={}ms", uid, cost_ms);
    return true;
}
