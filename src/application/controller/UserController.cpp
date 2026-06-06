// UserController.cpp - 用户注册、登录、重置密码业务实现
// 所有基础设施调用均通过接口委托，不直接依赖具体实现
#include "UserController.h"
#include "HttpConnection.h"
#include "IStatusRpcClient.h"
#include "IUserCache.h"
#include "IUserDao.h"
#include "IVerifyCodeCache.h"
#include "JsonUtil.h"
#include "Log.h"
#include "error_codes.h"

#include <json/value.h>

UserController::UserController(std::shared_ptr<IUserDao> userDao,
                               std::shared_ptr<IVerifyCodeCache> verifyCache,
                               std::shared_ptr<IUserCache> userCache,
                               std::shared_ptr<IStatusRpcClient> statusRpc)
    : _userDao(std::move(userDao)), _verifyCache(std::move(verifyCache)),
      _userCache(std::move(userCache)), _statusRpc(std::move(statusRpc))
{
}

void UserController::handleRegister(std::shared_ptr<HttpConnection> conn)
{
    Log::info(LogModule::Http, "UserController::handleRegister");

    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() || !src_root.isMember("email") || !src_root.isMember("verify_code") ||
        !src_root.isMember("user") || !src_root.isMember("passwd") || !src_root.isMember("confirm"))
    {
        Log::warn(LogModule::Http, "Register: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    Log::info(LogModule::Http, "Register request for email: {}", email);

    // 验证验证码 — 通过接口调用
    std::string verify_code;
    if (!_verifyCache->getVerifyCode(email, verify_code))
    {
        Log::warn(LogModule::Http, "Register: verify code expired for {}", email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_EXPIRED);
        return;
    }
    if (verify_code != src_root["verify_code"].asString())
    {
        Log::warn(LogModule::Http, "Register: verify code mismatch for {}", email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_CODE_ERROR);
        return;
    }

    auto name = src_root["user"].asString();
    auto passwd = src_root["passwd"].asString();
    auto confirm = src_root["confirm"].asString();

    // 检查用户是否已存在 — 通过接口调用
    if (_userCache->existsKey(name) || _userDao->userNameExists(name) ||
        _userDao->emailExists(email))
    {
        Log::warn(LogModule::Http, "Register: user already exists - name: {}, email: {}", name,
                  email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::USER_EXIST);
        return;
    }

    // 检查密码一致性
    if (passwd != confirm)
    {
        Log::warn(LogModule::Http, "Register: password mismatch");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_NOT_MATCH);
        return;
    }

    // 注册用户 — 通过接口调用
    int uid = _userDao->regUser(name, email, passwd);
    if (uid == 0)
    {
        Log::warn(LogModule::Http, "Register: user already exists (uid=0) - {}", name);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::USER_EXIST);
        return;
    }
    if (uid < 0)
    {
        Log::error(LogModule::Http, "Register: regUser failed, uid={}", uid);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::RPCFAILED);
        return;
    }

    // 缓存用户信息 — 通过接口调用
    _userCache->setUserEmail(name, email);
    Log::info(LogModule::Http, "Register success: uid={}, name={}, email={}", uid, name, email);

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = passwd;
    root["confirm"] = confirm;
    root["verify_code"] = src_root["verify_code"].asString();
    JsonUtil::makeJsonResponse(conn, root);
}

void UserController::handleLogin(std::shared_ptr<HttpConnection> conn)
{
    Log::info(LogModule::Http, "UserController::handleLogin");

    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() || !src_root.isMember("email") || !src_root.isMember("passwd"))
    {
        Log::warn(LogModule::Http, "Login: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    auto pwd = src_root["passwd"].asString();
    Log::info(LogModule::Http, "Login request for email: {}", email);

    // 校验密码 — 通过接口调用
    UserInfo userInfo;
    if (!_userDao->checkPwd(email, pwd, userInfo))
    {
        Log::warn(LogModule::Http, "Login: password mismatch for {}", email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_NOT_MATCH);
        return;
    }

    // 获取 ChatServer — 通过接口调用
    auto reply = _statusRpc->getChatServer(userInfo.uid);
    if (reply.error())
    {
        Log::error(LogModule::Http, "Login: getChatServer RPC failed for uid={}", userInfo.uid);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::RPCFAILED);
        return;
    }

    Log::info(LogModule::Http, "Login success: uid={}, email={}, host={}:{}", userInfo.uid, email,
              reply.host(), reply.port());

    Json::Value root;
    root["error"] = 0;
    root["email"] = email;
    root["uid"] = userInfo.uid;
    root["token"] = reply.token();
    root["host"] = reply.host();
    root["port"] = reply.port();
    JsonUtil::makeJsonResponse(conn, root);
}

void UserController::handleResetPwd(std::shared_ptr<HttpConnection> conn)
{
    Log::info(LogModule::Http, "UserController::handleResetPwd");

    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() || !src_root.isMember("email") || !src_root.isMember("user") ||
        !src_root.isMember("passwd") || !src_root.isMember("verify_code"))
    {
        Log::warn(LogModule::Http, "ResetPwd: invalid JSON fields");
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    auto name = src_root["user"].asString();
    auto pwd = src_root["passwd"].asString();
    Log::info(LogModule::Http, "ResetPwd request for email: {}, user: {}", email, name);

    // 验证验证码 — 通过接口调用
    std::string verify_code;
    if (!_verifyCache->getVerifyCode(email, verify_code))
    {
        Log::warn(LogModule::Http, "ResetPwd: verify code expired for {}", email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_EXPIRED);
        return;
    }
    if (verify_code != src_root["verify_code"].asString())
    {
        Log::warn(LogModule::Http, "ResetPwd: verify code mismatch for {}", email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_CODE_ERROR);
        return;
    }

    // 验证邮箱与用户匹配 — 通过接口调用
    if (!_userDao->checkEmail(email, name))
    {
        Log::warn(LogModule::Http, "ResetPwd: email {} not match user {}", email, name);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::EMAIL_NOT_MATCH);
        return;
    }

    // 更新密码 — 通过接口调用
    if (!_userDao->updatePwd(email, pwd))
    {
        Log::error(LogModule::Http, "ResetPwd: updatePwd failed for {}", email);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_UP_FAILED);
        return;
    }

    Log::info(LogModule::Http, "ResetPwd success: email={}", email);

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = pwd;
    JsonUtil::makeJsonResponse(conn, root);
}
