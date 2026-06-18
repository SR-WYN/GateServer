// VerifyCodeCacheImpl.h - VerifyCodeCache 适配器
// 将 RedisMgr 封装为 VerifyCodeCache 接口
#pragma once

#include "VerifyCodeCache.h"

/// VerifyCodeCache 的 Redis 实现适配器
class VerifyCodeCacheImpl : public VerifyCodeCache
{
public:
    bool getVerifyCode(const std::string &email, std::string &code) override;
};
