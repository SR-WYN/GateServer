add_library(gs_mysql STATIC
    MySqlPool.cpp
    MySqlMgr.cpp
    MySqlMgr.h
)

target_link_libraries(gs_mysql PUBLIC
    gateserver_includes
    gs_domain
    gs_base
    gs_config
    gs_log
    mysqlcppconn
)
