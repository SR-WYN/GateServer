// VerifyControllerImpl.cpp - HTTP 验证码控制器实现
#include "VerifyControllerImpl.h"

#include "HttpConnection.h"
#include "JsonUtil.h"
#include "Log.h"
#include "LogModule.h"
#include "VerifyService.h"
#include "error_codes.h"

VerifyControllerImpl::VerifyControllerImpl(std::shared_ptr<VerifyService> verifyService)
    : _verifyService(std::move(verifyService))
{
}

void VerifyControllerImpl::handleGetVerifyCode(std::shared_ptr<HttpConnection> conn)
{
    LOGI(LogModule::Http, "VerifyControllerImpl::handleGetVerifyCode");

    Json::Value srcRoot = JsonUtil::parseRequestBody(conn);
    if (srcRoot.isNull() || !srcRoot.isMember("email"))
    {
        LOGW(LogModule::Http, "GetVerifyCode: invalid JSON, missing email");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = srcRoot["email"].asString();
    auto rsp = _verifyService->getVerifyCode(email);

    LOGI(LogModule::Http, "GetVerifyCode response error={} for email: {}", rsp.error(), email);

    Json::Value root;
    root["error"] = rsp.error();
    root["email"] = email;
    JsonUtil::makeJsonResponse(conn, root);
}
