add_library(gs_redis STATIC
    RedisMgr.cpp
    UserCacheImpl.cpp
    VerifyCodeCacheImpl.cpp
)

target_link_libraries(gs_redis PUBLIC
    gateserver_includes
    gs_base
    gs_config
    gs_log
    redis++
    hiredis
    JsonCpp::JsonCpp
)
