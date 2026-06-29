// business_constants.h - 业务配置常量
#pragma once

namespace constants::business
{

inline constexpr int kSessionTtlSeconds = 300;      // 会话 TTL
inline constexpr int kTokenValiditySeconds = 300;   // token 有效期
inline constexpr int kUserCredTtlSeconds = 604800;  // 用户凭证缓存 7 天

} // namespace constants::business