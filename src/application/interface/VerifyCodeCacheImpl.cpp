// VerifyCodeCacheImpl.cpp - VerifyCodeCache 适配器实现
#include "VerifyCodeCacheImpl.h"
#include "RedisMgr.h"
#include "error_codes.h"

bool VerifyCodeCacheImpl::getVerifyCode(const std::string &email, std::string &code)
{
    std::string redis_key = std::string(RedisPrefix::CODE) + email;
    return RedisMgr::getInstance().get(redis_key, code);
}
