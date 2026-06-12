add_library(gs_application STATIC
    LogicSystem.cpp
    common/JsonUtil.cpp
    controller/UserController.cpp
    controller/VerifyController.cpp
    interface/UserDaoImpl.cpp
    interface/VerifyCodeCacheImpl.cpp
    interface/UserCacheImpl.cpp
    interface/StatusRpcClientImpl.cpp
    interface/VerifyRpcClientImpl.cpp
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
