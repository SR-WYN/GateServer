// VerifyCodeCache.h - 验证码缓存操作接口
// 抽象验证码的存储与校验，通常基于 Redis 实现
#pragma once

#include <string>

/// 验证码缓存接口
class VerifyCodeCache
{
public:
    virtual ~VerifyCodeCache() = default;

    /// 获取指定邮箱的验证码，返回是否找到
    virtual bool getVerifyCode(const std::string &email, std::string &code) = 0;
};
