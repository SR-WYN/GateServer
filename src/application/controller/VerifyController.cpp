// VerifyController.cpp - 获取验证码业务实现
// 所有基础设施调用均通过接口委托
#include "VerifyController.h"
#include "HttpConnection.h"
#include "IVerifyRpcClient.h"
#include "JsonUtil.h"
#include "Log.h"
#include "error_codes.h"

#include <json/value.h>

VerifyController::VerifyController(std::shared_ptr<IVerifyRpcClient> verifyRpc)
    : _verifyRpc(std::move(verifyRpc))
{
}

void VerifyController::handleGetVerifyCode(std::shared_ptr<HttpConnection> conn)
{
    Log::info(LogModule::Http, "VerifyController::handleGetVerifyCode");

    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() || !src_root.isMember("email"))
    {
        Log::warn(LogModule::Http, "GetVerifyCode: invalid JSON, missing email");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    Log::info(LogModule::Http, "GetVerifyCode request for email: {}", email);

    // 通过接口调用 gRPC 客户端
    GetVerifyRsp rsp = _verifyRpc->getVerifyCode(email);
    Log::info(LogModule::Http, "GetVerifyCode response error={} for email: {}", rsp.error(), email);

    Json::Value root;
    root["error"] = rsp.error();
    root["email"] = email;
    JsonUtil::makeJsonResponse(conn, root);
}
