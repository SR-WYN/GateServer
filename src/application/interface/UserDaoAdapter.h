// UserDaoAdapter.h - IUserDao 适配器
// 将 MysqlMgr 封装为 IUserDao 接口
#pragma once

#include "IUserDao.h"

/// IUserDao 的 MySQL 实现适配器
/// 内部委托给 MysqlMgr::getInstance().dao()
class UserDaoAdapter : public IUserDao {
public:
    int regUser(const std::string& name,
                const std::string& email,
                const std::string& pwd,
                const std::string& nick = "",
                int sex = 0) override;

    bool userNameExists(const std::string& name) override;
    bool emailExists(const std::string& email) override;
    bool checkEmail(const std::string& email,
                    const std::string& name) override;
    bool updatePwd(const std::string& email,
                   const std::string& pwd) override;
    bool checkPwd(const std::string& email,
                  const std::string& pwd,
                  UserInfo& userInfo) override;
};
