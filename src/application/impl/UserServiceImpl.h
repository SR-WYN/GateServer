// UserServiceImpl.h - 用户业务服务实现
#pragma once

#include "UserService.h"
#include <memory>

class UserDao;
class UserCache;
class VerifyCodeCache;
class StatusRpcClient;

class UserServiceImpl final : public UserService
{
public:
    UserServiceImpl(std::shared_ptr<UserDao> userDao, std::shared_ptr<UserCache> userCache,
                    std::shared_ptr<VerifyCodeCache> verifyCache,
                    std::shared_ptr<StatusRpcClient> statusRpc);

    int registerUser(const std::string &email, const std::string &verifyCode,
                     const std::string &name, const std::string &passwd, const std::string &confirm,
                     const std::string &nick, int sex, UserInfo &outUser) override;

    int loginUser(const std::string &email, const std::string &passwd, int &outUid,
                  std::string &outToken, std::string &outHost,
                  std::string &outPort) override;

    int resetPassword(const std::string &email, const std::string &verifyCode,
                      const std::string &name, const std::string &newPwd) override;

    void handleUserOffline(int uid) override;

private:
    std::shared_ptr<UserDao> _userDao;
    std::shared_ptr<UserCache> _userCache;
    std::shared_ptr<VerifyCodeCache> _verifyCache;
    std::shared_ptr<StatusRpcClient> _statusRpc;

    static constexpr int SESSION_TTL_SECONDS = 86400;    // 会话缓存 24 小时
    static constexpr int TOKEN_VALIDITY_SECONDS = 7200;  // Token 有效期 2 小时
    static constexpr int USER_CRED_TTL_SECONDS = 604800; // 用户凭证缓存 7 天

    static int64_t getCurrentTimestamp();
};
