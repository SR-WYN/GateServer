// VerifyController.h - 验证码相关业务
// 通过依赖注入获取 gRPC 客户端，不直接依赖具体实现
#pragma once

#include <memory>

class HttpConnection;
class IVerifyRpcClient;

class VerifyController {
public:
    explicit VerifyController(std::shared_ptr<IVerifyRpcClient> verifyRpc);
    void handleGetVerifyCode(std::shared_ptr<HttpConnection> conn);

private:
    std::shared_ptr<IVerifyRpcClient> _verifyRpc;
};
