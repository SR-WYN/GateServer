add_subdirectory(base)
add_subdirectory(log)
add_subdirectory(config)
add_subdirectory(net)
add_subdirectory(grpc)
add_subdirectory(mysql)
add_subdirectory(redis)

add_library(gs_infra INTERFACE)

target_link_libraries(gs_infra INTERFACE
    gateserver_includes
    common
    gs_domain
    gs_base
    gs_log
    gs_config
    gs_net
    gs_grpc
    gs_mysql
    gs_redis
    Boost::system
    Boost::thread
    Boost::filesystem
    JsonCpp::JsonCpp
    gRPC::grpc++
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    redis++
    hiredis
    mysqlcppconn
    spdlog::spdlog
)
