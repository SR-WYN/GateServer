add_library(gs_mysql STATIC MySqlPool.cpp MySqlDao.cpp MysqlMgr.cpp)
target_link_libraries(gs_mysql PUBLIC gateserver_includes gs_config gs_common mysqlcppconn)
