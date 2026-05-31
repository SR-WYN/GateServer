// UserController.cpp - 用户注册、登录、重置密码业务实现
#include "UserController.h"
#include "HttpConnection.h"
#include "JsonUtil.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "data.h"
#include "error_codes.h"

#include <json/value.h>

void UserController::handleRegister(std::shared_ptr<HttpConnection> conn)
{
    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() ||
        !src_root.isMember("email") || !src_root.isMember("verify_code") ||
        !src_root.isMember("user")   || !src_root.isMember("passwd") ||
        !src_root.isMember("confirm"))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    // 验证验证码
    std::string redis_key = std::string(RedisPrefix::CODE) + src_root["email"].asString();
    std::string verify_code;
    if (!RedisMgr::getInstance().get(redis_key, verify_code))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_EXPIRED);
        return;
    }
    if (verify_code != src_root["verify_code"].asString())
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_CODE_ERROR);
        return;
    }

    auto name  = src_root["user"].asString();
    auto email = src_root["email"].asString();
    auto passwd = src_root["passwd"].asString();
    auto confirm = src_root["confirm"].asString();

    // 检查用户是否已存在
    if (RedisMgr::getInstance().existsKey(name) ||
        MysqlMgr::getInstance().dao().userNameExists(name) ||
        MysqlMgr::getInstance().dao().emailExists(email))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::USER_EXIST);
        return;
    }

    // 检查密码一致性
    if (passwd != confirm)
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_NOT_MATCH);
        return;
    }

    // 注册用户
    int uid = MysqlMgr::getInstance().dao().regUser(name, email, passwd);
    if (uid == 0)
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::USER_EXIST);
        return;
    }
    if (uid < 0)
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::RPCFAILED);
        return;
    }

    RedisMgr::getInstance().set(name, email);

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
    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() || !src_root.isMember("email") || !src_root.isMember("passwd"))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    auto pwd   = src_root["passwd"].asString();

    UserInfo userInfo;
    if (!MysqlMgr::getInstance().dao().checkPwd(email, pwd, userInfo))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_NOT_MATCH);
        return;
    }

    auto reply = StatusGrpcClient::getInstance().getChatServer(userInfo.uid);
    if (reply.error())
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::RPCFAILED);
        return;
    }

    Json::Value root;
    root["error"] = 0;
    root["email"] = email;
    root["uid"]   = userInfo.uid;
    root["token"] = reply.token();
    root["host"]  = reply.host();
    root["port"]  = reply.port();
    JsonUtil::makeJsonResponse(conn, root);
}

void UserController::handleResetPwd(std::shared_ptr<HttpConnection> conn)
{
    Json::Value src_root = JsonUtil::parseRequestBody(conn);
    if (src_root.isNull() ||
        !src_root.isMember("email") || !src_root.isMember("user") ||
        !src_root.isMember("passwd") || !src_root.isMember("verify_code"))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::ERROR_JSON);
        return;
    }

    auto email = src_root["email"].asString();
    auto name  = src_root["user"].asString();
    auto pwd   = src_root["passwd"].asString();

    // 验证验证码
    std::string verify_code;
    if (!RedisMgr::getInstance().get(std::string(RedisPrefix::CODE) + email, verify_code))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_EXPIRED);
        return;
    }
    if (verify_code != src_root["verify_code"].asString())
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::VERIFY_CODE_ERROR);
        return;
    }

    // 验证邮箱与用户匹配
    if (!MysqlMgr::getInstance().dao().checkEmail(email, name))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::EMAIL_NOT_MATCH);
        return;
    }

    // 更新密码
    if (!MysqlMgr::getInstance().dao().updatePwd(email, pwd))
    {
        JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_UP_FAILED);
        return;
    }

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"]  = name;
    root["passwd"] = pwd;
    JsonUtil::makeJsonResponse(conn, root);
}
