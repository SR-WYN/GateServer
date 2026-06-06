// VerifyCodeCacheAdapter.h - IVerifyCodeCache 适配器
// 将 RedisMgr 封装为 IVerifyCodeCache 接口
#pragma once

#include "IVerifyCodeCache.h"

/// IVerifyCodeCache 的 Redis 实现适配器
class VerifyCodeCacheAdapter : public IVerifyCodeCache {
public:
    bool getVerifyCode(const std::string& email,
                       std::string& code) override;
};
