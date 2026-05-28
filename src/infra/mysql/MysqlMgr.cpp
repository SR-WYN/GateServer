#include "MysqlMgr.h"

MysqlMgr::MysqlMgr() = default;

MysqlMgr::~MysqlMgr() = default;

MySqlDao &MysqlMgr::dao()
{
    return _dao;
}
