// IUserDao.h - 用户数据持久化操作接口
// 抽象用户注册、登录、密码重置等数据库操作
#pragma once

#include "data.h"
#include <string>

/// 用户数据访问接口
/// 负责用户信息的持久化存储与查询
class IUserDao {
public:
    virtual ~IUserDao() = default;

    /// 注册新用户，返回 uid（>0 成功，0 已存在，<0 失败）
    virtual int regUser(const std::string& name,
                        const std::string& email,
                        const std::string& pwd,
                        const std::string& nick = "",
                        int sex = 0) = 0;

    /// 检查用户名是否已存在
    virtual bool userNameExists(const std::string& name) = 0;

    /// 检查邮箱是否已存在
    virtual bool emailExists(const std::string& email) = 0;

    /// 验证邮箱与用户名是否匹配
    virtual bool checkEmail(const std::string& email,
                            const std::string& name) = 0;

    /// 更新用户密码
    virtual bool updatePwd(const std::string& email,
                           const std::string& pwd) = 0;

    /// 校验密码，成功时填充 UserInfo
    virtual bool checkPwd(const std::string& email,
                          const std::string& pwd,
                          UserInfo& userInfo) = 0;
};
