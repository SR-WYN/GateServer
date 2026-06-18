add_library(gs_grpc STATIC
    VerifyGrpcClient.cpp
    StatusGrpcClient.cpp
    RPConPool.cpp
    StatusConPool.cpp
    StatusRpcClientImpl.cpp
    VerifyRpcClientImpl.cpp
)

add_dependencies(gs_grpc gateserver_proto_gen)

target_link_libraries(gs_grpc PUBLIC
    gateserver_includes
    gs_domain
    gs_base
    gs_config
    gs_log
    gRPC::grpc++
    protobuf::libprotobuf
)
