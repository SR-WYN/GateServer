add_library(gs_application STATIC
    interface/UserDao.h
    interface/UserService.h
    interface/VerifyService.h
    interface/UserController.h
    interface/VerifyController.h
    interface/LogicSystem.h
    impl/UserDaoImpl.cpp
    impl/UserServiceImpl.cpp
    impl/VerifyServiceImpl.cpp
    impl/UserControllerImpl.cpp
    impl/VerifyControllerImpl.cpp
    impl/LogicSystemImpl.cpp
    impl/GateService.cpp
)

target_link_libraries(gs_application PUBLIC
    gateserver_includes
    gs_domain
    gs_infra
    gRPC::grpc++
    protobuf::libprotobuf
)
