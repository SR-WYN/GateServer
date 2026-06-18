// GateService.cpp - GateServer gRPC 服务统一实现
#include "GateService.h"

#include "Log.h"
#include "LogModule.h"
#include "error_codes.h"

GateService::GateService(std::shared_ptr<UserService> userService)
    : _userService(std::move(userService))
{
}

grpc::Status GateService::NotifyUserOffline(grpc::ServerContext *context,
                                            const message::UserOfflineReq *request,
                                            message::UserOfflineRsp *reply)
{
    (void)context;

    int uid = request->uid();
    LOGI(LogModule::Grpc, "Received user offline notification, uid={}", uid);

    // 调用 UserService 处理下线逻辑
    _userService->handleUserOffline(uid);

    reply->set_error(ErrorCodes::SUCCESS);
    return grpc::Status::OK;
}
