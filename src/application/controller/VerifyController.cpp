// VerifyController.cpp - 获取验证码业务实现
#include "VerifyController.h"
#include "HttpConnection.h"
#include "JsonUtil.h"
#include "VerifyGrpcClient.h"
#include "error_codes.h"

#include <json/value.h>

void VerifyController::handleGetVerifyCode(std::shared_ptr<HttpConnection> conn)
{
    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() || !src_root.isMember("email"))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    GetVerifyRsp rsp = VerifyGrpcClient::getInstance().getVerifyCode(email);

    Json::Value root;
    root["error"] = rsp.error();
    root["email"] = email;
    JsonUtil::makeJsonResponse(conn, root);
}
