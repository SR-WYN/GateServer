// VerifyControllerImpl.cpp - HTTP 验证码控制器实现
#include "VerifyControllerImpl.h"

#include "HttpConnection.h"
#include "json_util.h"
#include "Log.h"
#include "LogModule.h"
#include "VerifyService.h"
#include "error_codes.h"

#include <chrono>

VerifyControllerImpl::VerifyControllerImpl(std::shared_ptr<VerifyService> verifyService)
    : _verifyService(std::move(verifyService))
{
}

void VerifyControllerImpl::handleGetVerifyCode(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "VerifyControllerImpl::handleGetVerifyCode");

    Json::Value srcRoot = JsonUtil::parseRequestBody(conn);
    if (srcRoot.isNull() || !srcRoot.isMember("email"))
    {
        LOGW(LogModule::Http, "GetVerifyCode: invalid JSON, missing email");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = srcRoot["email"].asString();
    LOGI(LogModule::Http, "GetVerifyCode: email={}", email);

    auto rsp = _verifyService->getVerifyCode(email);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();

    Json::Value root;
    root["error"] = rsp.error();
    root["email"] = email;
    JsonUtil::makeJsonResponse(conn, root);

    if (rsp.error() == ErrorCodes::SUCCESS)
    {
        LOGI(LogModule::Http, "GetVerifyCode success: email={} cost={}ms", email, cost_ms);
    }
    else
    {
        LOGW(LogModule::Http, "GetVerifyCode failed: email={} err={} cost={}ms", email,
             rsp.error(), cost_ms);
    }
}
