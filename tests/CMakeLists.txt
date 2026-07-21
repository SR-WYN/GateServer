add_executable(gateserver_tests
    test_utils.cpp
    test_logic_system.cpp
)

target_include_directories(gateserver_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common/utils
    ${CMAKE_SOURCE_DIR}/src/common/constants
    ${CMAKE_SOURCE_DIR}/src/domain
    ${CMAKE_SOURCE_DIR}/src/application/interface
    ${CMAKE_SOURCE_DIR}/src/application/impl
    ${CMAKE_SOURCE_DIR}/src/infra/base
    ${CMAKE_SOURCE_DIR}/src/infra/log
    ${CMAKE_SOURCE_DIR}/src/infra/config
    ${CMAKE_SOURCE_DIR}/src/infra/grpc
    ${CMAKE_SOURCE_DIR}/src/infra/mysql
    ${CMAKE_SOURCE_DIR}/src/infra/redis
    ${CMAKE_SOURCE_DIR}/src/infra/net
    ${PROTO_GEN_DIR}
)

target_link_libraries(gateserver_tests PRIVATE
    gs_application
    gs_infra
    common
    GTest::gtest_main
)

add_test(NAME UrlEncodeTest COMMAND gateserver_tests --gtest_filter=UrlEncodeTest*)
add_test(NAME UrlDecodeTest COMMAND gateserver_tests --gtest_filter=UrlDecodeTest*)
add_test(NAME ParseLevelTest COMMAND gateserver_tests --gtest_filter=ParseLevelTest*)
add_test(NAME DeferTest COMMAND gateserver_tests --gtest_filter=DeferTest*)
add_test(NAME LogicSystemTest COMMAND gateserver_tests --gtest_filter=GateServerLogicSystemTest*)
