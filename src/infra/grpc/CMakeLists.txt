add_library(gs_grpc STATIC
    VerifyGrpcClient.cpp
    StatusGrpcClient.cpp
    RPConPool.cpp
    StatusConPool.cpp
)
target_link_libraries(gs_grpc PUBLIC gateserver_includes gRPC::grpc++ protobuf::libprotobuf gs_config gs_common)
