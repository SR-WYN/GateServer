// GateService.h - GateServer gRPC 服务统一入口
// 供其他服务器调用，处理用户下线等通知
#pragma once

#include "UserController.h"

#include <grpcpp/grpcpp.h>
#include <message.grpc.pb.h>

#include <memory>

/// GateServer gRPC 服务统一实现
class GateService final : public message::GateNotifyService::Service {
public:
    /// 构造函数
    /// @param user_controller 用户控制器，用于处理下线逻辑
    explicit GateService(std::shared_ptr<UserController> user_controller);

    /// 处理用户下线通知
    /// @param context gRPC 上下文
    /// @param request 下线请求
    /// @param reply 响应
    /// @return gRPC 状态
    grpc::Status NotifyUserOffline(grpc::ServerContext* context,
                                   const message::UserOfflineReq* request,
                                   message::UserOfflineRsp* reply) override;

private:
    std::shared_ptr<UserController> _user_controller;
};
