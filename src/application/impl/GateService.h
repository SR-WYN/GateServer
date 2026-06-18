// GateService.h - GateServer gRPC 服务统一入口
// 供其他服务器调用，处理用户下线等通知
#pragma once

#include "UserService.h"

#include <grpcpp/grpcpp.h>
#include <message.grpc.pb.h>

#include <memory>

/// GateServer gRPC 服务统一实现
class GateService final : public message::GateNotifyService::Service
{
public:
    /// 构造函数
    /// @param userService 用户业务服务，用于处理下线逻辑
    explicit GateService(std::shared_ptr<UserService> userService);

    /// 处理用户下线通知
    grpc::Status NotifyUserOffline(grpc::ServerContext *context,
                                   const message::UserOfflineReq *request,
                                   message::UserOfflineRsp *reply) override;

private:
    std::shared_ptr<UserService> _userService;
};
