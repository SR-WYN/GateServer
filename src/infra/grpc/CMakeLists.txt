add_library(gs_grpc STATIC
    VerifyGrpcClient.cpp
    StatusGrpcClient.cpp
    RPConPool.cpp
    StatusConPool.cpp
    GateService.cpp
)
add_dependencies(gs_grpc gateserver_proto_gen)
target_link_libraries(gs_grpc PUBLIC gateserver_includes gRPC::grpc++ protobuf::libprotobuf gs_config gs_common)
