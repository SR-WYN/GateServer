// VerifyCodeCacheImpl.cpp - VerifyCodeCache 适配器实现
#include "VerifyCodeCacheImpl.h"
#include "Log.h"
#include "LogModule.h"
#include "RedisMgr.h"
#include "error_codes.h"
#include "redis_keys.h"

bool VerifyCodeCacheImpl::getVerifyCode(const std::string &email, std::string &code)
{
    std::string redis_key = std::string(constants::redis::kCodePrefix) + email;
    bool ok = RedisMgr::getInstance().get(redis_key, code);
    if (ok)
    {
        LOGI(LogModule::Redis, "VerifyCodeCache hit for email={}", email);
    }
    else
    {
        LOGI(LogModule::Redis, "VerifyCodeCache miss for email={}", email);
    }
    return ok;
}
