# 扁平 #include：各子目录均加入 INTERFACE，源文件写 "Foo.h" 即可

set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

add_library(gateserver_includes INTERFACE)
target_include_directories(gateserver_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}/application
    ${CMAKE_CURRENT_SOURCE_DIR}/application/common
    ${CMAKE_CURRENT_SOURCE_DIR}/application/controller
    ${CMAKE_CURRENT_SOURCE_DIR}/application/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/config
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/redis
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/mysql
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/grpc
    ${PROTO_GEN_DIR}
)

add_subdirectory(common)
add_subdirectory(domain)
add_subdirectory(infra/config)
add_subdirectory(infra/redis)
add_subdirectory(infra/mysql)
add_subdirectory(infra/grpc)
add_subdirectory(net)
add_subdirectory(application)
add_subdirectory(app)

add_executable(${PROJECT_NAME}
    app/GateServer.cpp
    ${PROTO_SRCS}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROTO_GEN_DIR})

# gs_net 中的 HttpConnection 依赖 LogicSystem；静态链接顺序在 gs_net 之后放 gs_application
target_link_libraries(${PROJECT_NAME} PRIVATE
    gateserver_includes
    gateserver_app_dir
    gs_net
    gs_application
    gs_common
    gs_config
    gs_redis
    gs_mysql
    gs_grpc
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.json"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.json"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/src/logdir"
    COMMENT "Copying config.json and creating src/logdir..."
)
