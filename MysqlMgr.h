#pragma once
#include "Singleton.h"
#include "MySqlDao.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;
public:
    ~MysqlMgr() override;
    int RegUser(const std::string& name,const std::string& email,const std::string& pwd);
private:
    MysqlMgr();
    MySqlDao _dao;
};