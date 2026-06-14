// UserController.cpp - 用户注册、登录、重置密码业务实现
// 所有基础设施调用均通过接口委托，不直接依赖具体实现
#include "UserController.h"

#include "HttpConnection.h"
#include "JsonUtil.h"
#include "Log.h"
#include "StatusRpcClient.h"
#include "UserCache.h"
#include "UserDao.h"
#include "VerifyCodeCache.h"
#include "error_codes.h"

#include <chrono>
#include <json/value.h>

UserController::UserController(std::shared_ptr<UserDao> userDao,
                               std::shared_ptr<VerifyCodeCache> verifyCache,
                               std::shared_ptr<UserCache> userCache,
                               std::shared_ptr<StatusRpcClient> statusRpc)
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
    auto nick = src_root.get("nick", "").asString();
    int sex = src_root.get("sex", 0).asInt();

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
    int uid = _userDao->regUser(name, email, passwd, nick, sex);
    if (uid == 0)
    {
        Log::warn(LogModule::Http, "Register: user already exists (uid=0) - {}", name);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::USER_EXIST);
        return;
    }
    if (uid < 0)
    {
        Log::error(LogModule::Http, "Register: regUser failed, uid={}", uid);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::RPC_FAILED);
        return;
    }

    // 缓存用户信息 — 通过接口调用
    _userCache->setUserEmail(name, email);

    // 预缓存用户凭证，使首次登录即可走 Redis
    _userCache->cacheUserCredential(email, uid, name, passwd, USER_CRED_TTL_SECONDS);

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

    UserInfo userInfo;
    bool from_cache = false;

    // 1. 优先尝试从 Redis 获取用户凭证并校验密码
    int cached_uid = 0;
    std::string cached_name;
    std::string cached_pwd_hash;
    if (_userCache->getUserCredential(email, cached_uid, cached_name, cached_pwd_hash))
    {
        if (cached_pwd_hash == pwd)
        {
            userInfo.uid = cached_uid;
            userInfo.name = cached_name;
            userInfo.email = email;
            userInfo.pwd = cached_pwd_hash;
            from_cache = true;

            Log::info(LogModule::Http, "Login credential cache hit: uid={}, email={}",
                      cached_uid, email);
        }
        else
        {
            Log::warn(LogModule::Http, "Login: password mismatch (Redis cache) for {}", email);
            JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_NOT_MATCH);
            return;
        }
    }

    // 2. Redis 缓存未命中，回源 MySQL 校验密码并获取用户信息
    if (!from_cache)
    {
        if (!_userDao->checkPwd(email, pwd, userInfo))
        {
            Log::warn(LogModule::Http, "Login: password mismatch (MySQL) for {}", email);
            JsonUtil::makeErrorResponse(conn, ErrorCodes::PASSWD_NOT_MATCH);
            return;
        }

        // 3. MySQL 校验成功，将用户凭证写入 Redis 缓存
        _userCache->cacheUserCredential(email, userInfo.uid, userInfo.name, pwd,
                                        USER_CRED_TTL_SECONDS);
        Log::info(LogModule::Http, "User credential cached for {}", email);
    }

    // 4. 原有会话缓存逻辑保持不变
    auto cachedSession = _userCache->getSession(userInfo.uid);

    if (cachedSession)
    {
        // 4.1 会话缓存命中 - 检查 Token 是否过期
        if (!cachedSession->_token.empty() && cachedSession->_expire_at > getCurrentTimestamp())
        {
            // Token 有效，延长缓存时间并直接返回
            _userCache->extendSession(userInfo.uid, SESSION_TTL_SECONDS);

            Log::info(LogModule::Http, "Login session cache hit: uid={}, email={}, host={}:{}",
                      userInfo.uid, email, cachedSession->_chat_host, cachedSession->_chat_port);

            Json::Value root;
            root["error"] = ErrorCodes::SUCCESS;
            root["email"] = email;
            root["uid"] = userInfo.uid;
            root["token"] = cachedSession->_token;
            root["host"] = cachedSession->_chat_host;
            root["port"] = cachedSession->_chat_port;
            JsonUtil::makeJsonResponse(conn, root);
            return;
        }

        // Token 已过期，删除缓存
        _userCache->removeSession(userInfo.uid);
    }

    // 4.2 会话缓存未命中或已过期 - 调用 StatusServer
    auto reply = _statusRpc->getChatServer(userInfo.uid);
    if (reply.error())
    {
        Log::error(LogModule::Http, "Login: getChatServer RPC failed for uid={}", userInfo.uid);
        JsonUtil::makeErrorResponse(conn, ErrorCodes::RPC_FAILED);
        return;
    }

    // 5. 写入 Redis 会话缓存
    UserSession session;
    session._uid = userInfo.uid;
    session._token = reply.token();
    session._email = email;
    session._user_name = userInfo.name;
    session._chat_host = reply.host();
    session._chat_port = reply.port();
    session._login_time = getCurrentTimestamp();
    session._expire_at = session._login_time + TOKEN_VALIDITY_SECONDS;

    _userCache->saveSession(session, SESSION_TTL_SECONDS);

    Log::info(LogModule::Http, "Login session cache miss: uid={}, email={}, host={}:{}",
              userInfo.uid, email, reply.host(), reply.port());

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
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

    // 使 Redis 用户凭证缓存失效
    _userCache->invalidateUserCredential(email);
    Log::info(LogModule::Http, "User credential cache invalidated for {}", email);

    Log::info(LogModule::Http, "ResetPwd success: email={}", email);

    Json::Value root;
    root["error"] = ErrorCodes::SUCCESS;
    root["email"] = email;
    root["user"] = name;
    root["passwd"] = pwd;
    JsonUtil::makeJsonResponse(conn, root);
}

void UserController::handleUserOffline(int uid)
{
    Log::info(LogModule::Http, "User offline notified, uid={}", uid);

    // 清理 Redis 缓存
    if (_userCache->isOnline(uid))
    {
        _userCache->removeSession(uid);
        Log::info(LogModule::Http, "Session cache removed for uid={}", uid);
    }
}

int64_t UserController::getCurrentTimestamp()
{
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}
