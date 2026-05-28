#pragma once
#include "MySqlDao.h"
#include "Singleton.h"

class MysqlMgr : public Singleton<MysqlMgr>
{
    friend class Singleton<MysqlMgr>;

public:
    ~MysqlMgr() override;
    MySqlDao &dao();

private:
    MysqlMgr();
    MySqlDao _dao;
};
