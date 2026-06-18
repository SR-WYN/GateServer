# 扁平 #include：各子目录均加入 INTERFACE，源文件写 "Foo.h" 即可

set_source_files_properties(${PROTO_SRCS} PROPERTIES GENERATED TRUE)

add_library(gateserver_includes INTERFACE)
target_include_directories(gateserver_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/application
    ${CMAKE_CURRENT_SOURCE_DIR}/application/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/application/impl
    ${CMAKE_CURRENT_SOURCE_DIR}/infra
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/base
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/log
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/config
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/grpc
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/mysql
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/redis
    ${CMAKE_CURRENT_SOURCE_DIR}/infra/net
    ${PROTO_GEN_DIR}
)

add_subdirectory(domain)
add_subdirectory(infra)
add_subdirectory(application)
add_subdirectory(app)

add_executable(${PROJECT_NAME}
    app/GateServer.cpp
    ${PROTO_SRCS}
)

# 固定可执行文件输出到 build 根目录，与 launch.json 保持一致
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
)

# 确保 proto 文件先生成，再编译可执行文件
add_dependencies(${PROJECT_NAME} gateserver_proto_gen)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROTO_GEN_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    gateserver_includes
    gateserver_app_dir
    gs_application
    gs_infra
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
