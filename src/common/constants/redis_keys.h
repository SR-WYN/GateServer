// redis_keys.h - Redis key 前缀常量
#pragma once

namespace constants::redis
{

constexpr const char *kCodePrefix = "code_";
constexpr const char *kSessionPrefix = "gate:session:";
constexpr const char *kUserCredPrefix = "gate:user_cred:";

} // namespace constants::redis