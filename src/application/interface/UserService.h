// UserService.h - 用户业务服务接口
// 负责注册、登录、重置密码、用户下线等用例编排
#pragma once

#include "data.h"
#include "message.pb.h"
#include <string>

class UserService
{
public:
    virtual ~UserService() = default;

    /// 用户注册
    /// @return 错误码（SUCCESS 表示成功）
    virtual int registerUser(const std::string &email, const std::string &verifyCode,
                             const std::string &name, const std::string &passwd,
                             const std::string &confirm, const std::string &nick, int sex,
                             UserInfo &outUser) = 0;

    /// 用户登录
    /// @return 错误码，成功时 outToken / outHost / outPort 被填充
    virtual int loginUser(const std::string &email, const std::string &passwd,
                          std::string &outToken, std::string &outHost, std::string &outPort) = 0;

    /// 重置密码
    /// @return 错误码（SUCCESS 表示成功）
    virtual int resetPassword(const std::string &email, const std::string &verifyCode,
                              const std::string &name, const std::string &newPwd) = 0;

    /// 处理用户下线通知（供 gRPC 入口调用）
    virtual void handleUserOffline(int uid) = 0;
};
