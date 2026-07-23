// GateService.cpp - GateServer gRPC 服务统一实现
#include "GateService.h"

#include "Log.h"
#include "LogModule.h"
#include "error_codes.h"

#include <chrono>

GateService::GateService(std::shared_ptr<UserService> userService)
    : _userService(std::move(userService))
{
}

grpc::Status GateService::NotifyUserOffline(grpc::ServerContext *context,
                                            const message::UserOfflineReq *request,
                                            message::UserOfflineRsp *reply)
{
    (void)context;
    const auto start = std::chrono::steady_clock::now();

    int uid = request->uid();
    LOGI(LogModule::Grpc, "Received user offline notification, uid={}", uid);

    // 调用 UserService 处理下线逻辑
    _userService->handleUserOffline(uid);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    LOGI(LogModule::Grpc, "NotifyUserOffline processed uid={} cost={}ms", uid, cost_ms);

    reply->set_error(ErrorCodes::SUCCESS);
    return grpc::Status::OK;
}

grpc::Status GateService::NotifyUserOnline(grpc::ServerContext *context,
                                           const message::UserOnlineReq *request,
                                           message::UserOnlineRsp *reply)
{
    (void)context;
    const auto start = std::chrono::steady_clock::now();

    int uid = request->uid();
    LOGI(LogModule::Grpc, "Received user online notification, uid={}", uid);

    // 调用 UserService 刷新 session TTL
    _userService->handleUserOnline(uid);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    LOGI(LogModule::Grpc, "NotifyUserOnline processed uid={} cost={}ms", uid, cost_ms);

    reply->set_error(ErrorCodes::SUCCESS);
    return grpc::Status::OK;
}
