// UserControllerImpl.cpp - HTTP 用户控制器实现
// 薄层：解析 HTTP 请求，调用 UserService，构造 JSON 响应
#include "UserControllerImpl.h"

#include "HttpConnection.h"
#include "JsonUtil.h"
#include "Log.h"
#include "LogModule.h"
#include "UserService.h"
#include "data.h"
#include "error_codes.h"

UserControllerImpl::UserControllerImpl(std::shared_ptr<UserService> userService)
    : _userService(std::move(userService))
{
}

void UserControllerImpl::handleRegister(std::shared_ptr<HttpConnection> conn)
{
    LOGI(LogModule::Http, "UserControllerImpl::handleRegister");

    Json::Value srcRoot = JsonUtil::parseRequestBody(conn);
    if (srcRoot.isNull() || !srcRoot.isMember("email") || !srcRoot.isMember("verify_code") ||
        !srcRoot.isMember("user") || !srcRoot.isMember("passwd") || !srcRoot.isMember("confirm"))
    {
        LOGW(LogModule::Http, "Register: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = srcRoot["email"].asString();
    auto verifyCode = srcRoot["verify_code"].asString();
    auto name = srcRoot["user"].asString();
    auto passwd = srcRoot["passwd"].asString();
    auto confirm = srcRoot["confirm"].asString();
    auto nick = srcRoot.get("nick", "").asString();
    int sex = srcRoot.get("sex", 0).asInt();

    UserInfo userInfo;
    int err =
        _userService->registerUser(email, verifyCode, name, passwd, confirm, nick, sex, userInfo);
    if (err != ErrorCodes::SUCCESS)
    {
        JsonUtil::makeErrorResponse(conn, err);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = passwd;
    root["confirm"] = confirm;
    root["verify_code"] = verifyCode;
    JsonUtil::makeJsonResponse(conn, root);
}

void UserControllerImpl::handleLogin(std::shared_ptr<HttpConnection> conn)
{
    LOGI(LogModule::Http, "UserControllerImpl::handleLogin");

    Json::Value srcRoot = JsonUtil::parseRequestBody(conn);
    if (srcRoot.isNull() || !srcRoot.isMember("email") || !srcRoot.isMember("passwd"))
    {
        LOGW(LogModule::Http, "Login: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = srcRoot["email"].asString();
    auto passwd = srcRoot["passwd"].asString();

    std::string token;
    std::string host;
    std::string port;
    int err = _userService->loginUser(email, passwd, token, host, port);
    if (err != ErrorCodes::SUCCESS)
    {
        JsonUtil::makeErrorResponse(conn, err);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["token"] = token;
    root["host"] = host;
    root["port"] = port;
    JsonUtil::makeJsonResponse(conn, root);
}

void UserControllerImpl::handleResetPwd(std::shared_ptr<HttpConnection> conn)
{
    LOGI(LogModule::Http, "UserControllerImpl::handleResetPwd");

    Json::Value srcRoot = JsonUtil::parseRequestBody(conn);
    if (srcRoot.isNull() || !srcRoot.isMember("email") || !srcRoot.isMember("user") ||
        !srcRoot.isMember("passwd") || !srcRoot.isMember("verify_code"))
    {
        LOGW(LogModule::Http, "ResetPwd: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = srcRoot["email"].asString();
    auto name = srcRoot["user"].asString();
    auto passwd = srcRoot["passwd"].asString();
    auto verifyCode = srcRoot["verify_code"].asString();

    int err = _userService->resetPassword(email, verifyCode, name, passwd);
    if (err != ErrorCodes::SUCCESS)
    {
        JsonUtil::makeErrorResponse(conn, err);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = passwd;
    JsonUtil::makeJsonResponse(conn, root);
}
