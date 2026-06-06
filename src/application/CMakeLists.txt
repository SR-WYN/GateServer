add_library(gs_application STATIC
    LogicSystem.cpp
    common/JsonUtil.cpp
    controller/UserController.cpp
    controller/VerifyController.cpp
    interface/UserDaoAdapter.cpp
    interface/VerifyCodeCacheAdapter.cpp
    interface/UserCacheAdapter.cpp
    interface/StatusRpcClientAdapter.cpp
    interface/VerifyRpcClientAdapter.cpp
)
target_link_libraries(gs_application PUBLIC
    gateserver_includes
    gs_domain
    gs_net
    gs_common
    gs_config
    gs_redis
    gs_mysql
    gs_grpc
)
