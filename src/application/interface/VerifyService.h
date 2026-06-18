// VerifyService.h - 验证码业务服务接口
#pragma once

#include "message.pb.h"
#include <string>

class VerifyService
{
public:
    virtual ~VerifyService() = default;

    /// 请求向指定邮箱发送验证码
    virtual message::GetVerifyRsp getVerifyCode(const std::string &email) = 0;
};
