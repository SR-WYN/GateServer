// redis_keys.h - Redis key 前缀常量
#pragma once

namespace constants::redis
{

constexpr const char *kSessionPrefix = "gate:session:";
constexpr const char *kNamePrefix = "gate:name:";
constexpr const char *kEmailPrefix = "gate:email:";
constexpr const char *kTokenPrefix = "gate:token:";
constexpr const char *kOnlineUsersKey = "gate:online_users";
constexpr const char *kUserCredPrefix = "gate:user_cred:";

} // namespace constants::redis