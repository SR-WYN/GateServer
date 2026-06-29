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

#include <chrono>

UserControllerImpl::UserControllerImpl(std::shared_ptr<UserService> userService)
    : _userService(std::move(userService))
{
}

void UserControllerImpl::handleRegister(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
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

    LOGI(LogModule::Http, "Register: email={} user={} nick={} sex={}", email, name, nick, sex);

    UserInfo userInfo;
    int err =
        _userService->registerUser(email, verifyCode, name, passwd, confirm, nick, sex, userInfo);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (err != ErrorCodes::SUCCESS)
    {
        LOGW(LogModule::Http, "Register failed: email={} err={} cost={}ms", email, err, cost_ms);
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

    LOGI(LogModule::Http, "Register success: uid={} email={} user={} cost={}ms", userInfo.uid,
         email, name, cost_ms);
}

void UserControllerImpl::handleLogin(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
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

    LOGI(LogModule::Http, "Login: email={}", email);

    int uid = 0;
    std::string token;
    std::string host;
    std::string port;
    int err = _userService->loginUser(email, passwd, uid, token, host, port);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (err != ErrorCodes::SUCCESS)
    {
        LOGW(LogModule::Http, "Login failed: email={} err={} cost={}ms", email, err, cost_ms);
        JsonUtil::makeErrorResponse(conn, err);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["uid"] = uid;
    root["email"] = email;
    root["token"] = token;
    root["host"] = host;
    root["port"] = port;
    JsonUtil::makeJsonResponse(conn, root);

    LOGI(LogModule::Http, "Login success: uid={} email={} host={}:{} cost={}ms", uid, email, host,
         port, cost_ms);
}

void UserControllerImpl::handleResetPwd(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
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

    LOGI(LogModule::Http, "ResetPwd: email={} user={}", email, name);

    int err = _userService->resetPassword(email, verifyCode, name, passwd);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (err != ErrorCodes::SUCCESS)
    {
        LOGW(LogModule::Http, "ResetPwd failed: email={} err={} cost={}ms", email, err, cost_ms);
        JsonUtil::makeErrorResponse(conn, err);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = passwd;
    JsonUtil::makeJsonResponse(conn, root);

    LOGI(LogModule::Http, "ResetPwd success: email={} cost={}ms", email, cost_ms);
}

void UserControllerImpl::handleLogout(std::shared_ptr<HttpConnection> conn)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "UserControllerImpl::handleLogout");

    Json::Value srcRoot = JsonUtil::parseRequestBody(conn);
    if (srcRoot.isNull() || !srcRoot.isMember("uid"))
    {
        LOGW(LogModule::Http, "Logout: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    int uid = srcRoot["uid"].asInt();
    LOGI(LogModule::Http, "Logout: uid={}", uid);

    int err = _userService->logoutUser(uid);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    if (err != ErrorCodes::SUCCESS)
    {
        LOGW(LogModule::Http, "Logout failed: uid={} err={} cost={}ms", uid, err, cost_ms);
        JsonUtil::makeErrorResponse(conn, err);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["uid"] = uid;
    JsonUtil::makeJsonResponse(conn, root);

    LOGI(LogModule::Http, "Logout success: uid={} cost={}ms", uid, cost_ms);
}
