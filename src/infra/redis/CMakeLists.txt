add_library(gs_redis STATIC RedisMgr.cpp)
target_link_libraries(gs_redis PUBLIC gateserver_includes gs_config redis++)
