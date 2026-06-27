// UserServiceImpl.cpp - 用户业务服务实现
#include "UserServiceImpl.h"

#include "Log.h"
#include "LogModule.h"
#include "StatusRpcClient.h"
#include "ThreadPoolMgr.h"
#include "UserCache.h"
#include "UserDao.h"
#include "VerifyCodeCache.h"
#include "error_codes.h"

#include <chrono>

UserServiceImpl::UserServiceImpl(std::shared_ptr<UserDao> userDao,
                                 std::shared_ptr<UserCache> userCache,
                                 std::shared_ptr<VerifyCodeCache> verifyCache,
                                 std::shared_ptr<StatusRpcClient> statusRpc)
    : _userDao(std::move(userDao)), _userCache(std::move(userCache)),
      _verifyCache(std::move(verifyCache)), _statusRpc(std::move(statusRpc))
{
}

int UserServiceImpl::registerUser(const std::string &email, const std::string &verifyCode,
                                  const std::string &name, const std::string &passwd,
                                  const std::string &confirm, const std::string &nick, int sex,
                                  UserInfo &outUser)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "UserServiceImpl::registerUser | email={} user={}", email, name);

    // 验证验证码
    std::string cachedCode;
    if (!_verifyCache->getVerifyCode(email, cachedCode))
    {
        LOGW(LogModule::Http, "Register: verify code expired for {}", email);
        return ErrorCodes::VERIFY_EXPIRED;
    }
    if (cachedCode != verifyCode)
    {
        LOGW(LogModule::Http, "Register: verify code mismatch for {} (input={}, cached={})",
             email, verifyCode, cachedCode);
        return ErrorCodes::VERIFY_CODE_ERROR;
    }

    // 检查用户是否已存在
    if (_userCache->existsKey(name) || _userDao->userNameExists(name) ||
        _userDao->emailExists(email))
    {
        LOGW(LogModule::Http, "Register: user already exists - name: {}, email: {}", name, email);
        return ErrorCodes::USER_EXIST;
    }

    // 检查密码一致性
    if (passwd != confirm)
    {
        LOGW(LogModule::Http, "Register: password mismatch for {}", email);
        return ErrorCodes::PASSWD_NOT_MATCH;
    }

    // 注册用户
    int uid = _userDao->regUser(name, email, passwd, nick, sex);
    if (uid == 0)
    {
        LOGW(LogModule::Http, "Register: user already exists (uid=0) - {}", name);
        return ErrorCodes::USER_EXIST;
    }
    if (uid < 0)
    {
        LOGE(LogModule::Http, "Register: regUser failed, uid={} email={}", uid, email);
        return ErrorCodes::RPC_FAILED;
    }

    // 缓存用户信息
    _userCache->setUserEmail(name, email);
    _userCache->cacheUserCredential(email, uid, name, passwd, USER_CRED_TTL_SECONDS);

    outUser.uid = uid;
    outUser.name = name;
    outUser.email = email;
    outUser.pwd = passwd;

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    LOGI(LogModule::Http, "Register success: uid={}, name={}, email={} cost={}ms", uid, name,
         email, cost_ms);
    return ErrorCodes::SUCCESS;
}

int UserServiceImpl::loginUser(const std::string &email, const std::string &passwd, int &outUid,
                               std::string &outToken, std::string &outHost,
                               std::string &outPort)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "UserServiceImpl::loginUser | email={}", email);

    UserInfo userInfo;
    bool fromCache = false;

    // 1. 优先尝试从 Redis 获取用户凭证并校验密码
    int cachedUid = 0;
    std::string cachedName;
    std::string cachedPwdHash;
    if (_userCache->getUserCredential(email, cachedUid, cachedName, cachedPwdHash))
    {
        if (cachedPwdHash == passwd)
        {
            userInfo.uid = cachedUid;
            userInfo.name = cachedName;
            userInfo.email = email;
            userInfo.pwd = cachedPwdHash;
            fromCache = true;

            LOGI(LogModule::Http, "Login credential cache hit: uid={}, email={}", cachedUid,
                 email);
        }
        else
        {
            LOGW(LogModule::Http, "Login: password mismatch (Redis cache) for {}", email);
            return ErrorCodes::PASSWD_NOT_MATCH;
        }
    }

    // 2. Redis 缓存未命中，回源 MySQL 校验密码并获取用户信息
    if (!fromCache)
    {
        LOGI(LogModule::Http, "Login credential cache miss, fallback to MySQL: email={}", email);
        if (!_userDao->checkPwd(email, passwd, userInfo))
        {
            LOGW(LogModule::Http, "Login: password mismatch (MySQL) for {}", email);
            return ErrorCodes::PASSWD_NOT_MATCH;
        }

        // 3. MySQL 校验成功，将用户凭证写入 Redis 缓存
        _userCache->cacheUserCredential(email, userInfo.uid, userInfo.name, passwd,
                                        USER_CRED_TTL_SECONDS);
        LOGI(LogModule::Http, "User credential cached for {} uid={}", email, userInfo.uid);
    }

    // 4. 原有会话缓存逻辑保持不变
    auto cachedSession = _userCache->getSession(userInfo.uid);
    if (cachedSession)
    {
        // 4.1 会话缓存命中 - 检查 Token 是否过期且仍被 StatusServer 承认
        if (!cachedSession->_token.empty() && cachedSession->_expire_at > getCurrentTimestamp())
        {
            int token_err = _statusRpc->validateToken(userInfo.uid, cachedSession->_token);
            if (token_err == ErrorCodes::SUCCESS)
            {
                // Token 有效，延长缓存时间并直接返回
                _userCache->extendSession(userInfo.uid, SESSION_TTL_SECONDS);

                const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                         std::chrono::steady_clock::now() - start)
                                         .count();
                LOGI(LogModule::Http,
                     "Login session cache hit: uid={}, email={}, host={}:{} token={} cost={}ms",
                     userInfo.uid, email, cachedSession->_chat_host, cachedSession->_chat_port,
                     cachedSession->_token, cost_ms);

                outUid = userInfo.uid;
                outToken = cachedSession->_token;
                outHost = cachedSession->_chat_host;
                outPort = cachedSession->_chat_port;
                return ErrorCodes::SUCCESS;
            }

            LOGW(LogModule::Http,
                 "Login session cache token invalid, refresh token: uid={}, email={}, err={}",
                 userInfo.uid, email, token_err);
        }
        else
        {
            LOGI(LogModule::Http,
                 "Login session cache expired: uid={}, email={}, token_empty={}", userInfo.uid,
                 email, cachedSession->_token.empty());
        }

        // Token 已过期或被 StatusServer 判定无效，删除缓存
        _userCache->removeSession(userInfo.uid);
    }

    // 4.2 会话缓存未命中或已过期 - 调用 StatusServer
    LOGI(LogModule::Http, "Login session cache miss, fetch ChatServer from StatusServer: uid={}",
         userInfo.uid);
    auto reply = _statusRpc->getChatServer(userInfo.uid);
    if (reply.error())
    {
        LOGE(LogModule::Http, "Login: getChatServer RPC failed for uid={}", userInfo.uid);
        return ErrorCodes::RPC_FAILED;
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

    if (!_userCache->saveSession(session, SESSION_TTL_SECONDS))
    {
        LOGE(LogModule::Http, "Login: saveSession failed for uid={}", userInfo.uid);
        return ErrorCodes::RPC_FAILED;
    }

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    LOGI(LogModule::Http,
         "Login session cache miss: uid={}, email={}, host={}:{} token={} cost={}ms",
         userInfo.uid, email, reply.host(), reply.port(), reply.token(), cost_ms);

    outUid = userInfo.uid;
    outToken = reply.token();
    outHost = reply.host();
    outPort = reply.port();
    return ErrorCodes::SUCCESS;
}

int UserServiceImpl::resetPassword(const std::string &email, const std::string &verifyCode,
                                   const std::string &name, const std::string &newPwd)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "UserServiceImpl::resetPassword | email={}, user={}", email, name);

    // 验证验证码
    std::string cachedCode;
    if (!_verifyCache->getVerifyCode(email, cachedCode))
    {
        LOGW(LogModule::Http, "ResetPwd: verify code expired for {}", email);
        return ErrorCodes::VERIFY_EXPIRED;
    }
    if (cachedCode != verifyCode)
    {
        LOGW(LogModule::Http, "ResetPwd: verify code mismatch for {} (input={}, cached={})",
             email, verifyCode, cachedCode);
        return ErrorCodes::VERIFY_CODE_ERROR;
    }

    // 验证邮箱与用户匹配
    if (!_userDao->checkEmail(email, name))
    {
        LOGW(LogModule::Http, "ResetPwd: email {} not match user {}", email, name);
        return ErrorCodes::EMAIL_NOT_MATCH;
    }

    // 更新密码
    if (!_userDao->updatePwd(email, newPwd))
    {
        LOGE(LogModule::Http, "ResetPwd: updatePwd failed for {}", email);
        return ErrorCodes::PASSWD_UP_FAILED;
    }

    // 使 Redis 用户凭证缓存失效
    _userCache->invalidateUserCredential(email);
    LOGI(LogModule::Http, "User credential cache invalidated for {}", email);

    const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    LOGI(LogModule::Http, "ResetPwd success: email={} cost={}ms", email, cost_ms);
    return ErrorCodes::SUCCESS;
}

void UserServiceImpl::handleUserOffline(int uid)
{
    const auto start = std::chrono::steady_clock::now();
    LOGI(LogModule::Http, "User offline notified, uid={}", uid);

    // 清理 Redis 缓存（投递到 Redis 线程池，避免阻塞 gRPC 回调线程）
    ThreadPoolMgr::getInstance().enqueueRedis([this, uid]() {
        if (_userCache->isOnline(uid))
        {
            if (_userCache->removeSession(uid))
            {
                LOGI(LogModule::Http, "Session cache removed for uid={}", uid);
            }
            else
            {
                LOGE(LogModule::Http, "Session cache remove failed for uid={}", uid);
            }
        }
        else
        {
            LOGI(LogModule::Http, "User not online, skip session removal uid={}", uid);
        }
    });
}

int64_t UserServiceImpl::getCurrentTimestamp()
{
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}
