// GateServer.cpp - 网关服务器入口
// 负责读取配置、组装依赖、启动 HTTP + gRPC 服务、信号处理
#include "CServer.h"
#include "ConfigMgr.h"
#include "GateService.h"
#include "HttpConnection.h"
#include "json_util.h"
#include "Log.h"
#include "LogModule.h"
#include "LogicSystem.h"
#include "LogicSystemImpl.h"
#include "MySqlMgr.h"
#include "MySqlPool.h"
#include "RedisMgr.h"
#include "StatusRpcClient.h"
#include "StatusRpcClientImpl.h"
#include "ThreadPoolMgr.h"
#include "UserCache.h"
#include "UserCacheImpl.h"
#include "UserController.h"
#include "UserControllerImpl.h"
#include "UserDao.h"
#include "UserDaoImpl.h"
#include "UserService.h"
#include "UserServiceImpl.h"
#include "VerifyCodeCache.h"
#include "VerifyCodeCacheImpl.h"
#include "VerifyController.h"
#include "VerifyControllerImpl.h"
#include "VerifyRpcClient.h"
#include "VerifyRpcClientImpl.h"
#include "VerifyService.h"
#include "VerifyServiceImpl.h"

#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <iostream>

namespace net = boost::asio;

static std::atomic<bool> g_quit{false};

static void signalHandler(int signal)
{
    LOGI(LogModule::App, "Received signal {}, shutting down...", signal);
    g_quit.store(true);
}

int main()
{
    try
    {
        if (!Log::init("GateServer", ConfigMgr::getInstance().getLogConfig()))
        {
            return 0;
        }
        ConfigMgr::getInstance();
        LOGI(LogModule::App, "GateServer starting");

        // 1. 初始化 MySQL 连接池并创建 DAO
        MySqlPool::initOnce();
        auto mysql_mgr = std::make_shared<MySqlMgr>();
        auto userDao = std::make_shared<UserDaoImpl>(mysql_mgr);
        auto userCache = std::make_shared<UserCacheImpl>();
        auto verifyCache = std::make_shared<VerifyCodeCacheImpl>();
        auto statusRpc = std::make_shared<StatusRpcClientImpl>();
        auto verifyRpc = std::make_shared<VerifyRpcClientImpl>();

        // 2. 创建应用服务
        auto userService =
            std::make_shared<UserServiceImpl>(userDao, userCache, verifyCache, statusRpc);
        auto verifyService = std::make_shared<VerifyServiceImpl>(verifyRpc);

        // 3. 创建 HTTP 控制器
        auto userController = std::make_shared<UserControllerImpl>(userService);
        auto verifyController = std::make_shared<VerifyControllerImpl>(verifyService);

        // 4. 创建路由器并注册路由
        auto logicSystem = std::make_shared<LogicSystemImpl>();
        logicSystem->regPost("/get_verify_code",
                             [verifyController](std::shared_ptr<HttpConnection> conn) {
                                 verifyController->handleGetVerifyCode(conn);
                             });
        logicSystem->regPost("/user_register",
                             [userController](std::shared_ptr<HttpConnection> conn) {
                                 userController->handleRegister(conn);
                             });
        logicSystem->regPost("/user_login", [userController](std::shared_ptr<HttpConnection> conn) {
            userController->handleLogin(conn);
        });
        logicSystem->regPost("/reset_pwd", [userController](std::shared_ptr<HttpConnection> conn) {
            userController->handleResetPwd(conn);
        });
        logicSystem->regPost("/user_logout",
                             [userController](std::shared_ptr<HttpConnection> conn) {
                                 userController->handleLogout(conn);
                             });

        // 5. 读取网关端口
        auto &config = ConfigMgr::getInstance();
        std::string gatePortStr = config["GateServer"]["Port"];
        unsigned short gatePort = static_cast<unsigned short>(std::stoi(gatePortStr));
        LOGI(LogModule::App, "GateServer HTTP port: {}", gatePort);

        // 6. 创建并启动 gRPC 服务
        std::string grpcPortStr = config["GateServer"]["GrpcPort"];
        if (grpcPortStr.empty())
        {
            grpcPortStr = "51052";
        }
        std::string grpcAddress = "0.0.0.0:" + grpcPortStr;
        LOGI(LogModule::App, "GateServer gRPC address: {}", grpcAddress);

        GateService gateService(userService);
        grpc::ServerBuilder grpcBuilder;
        grpcBuilder.AddListeningPort(grpcAddress, grpc::InsecureServerCredentials());
        grpcBuilder.RegisterService(&gateService);
        std::unique_ptr<grpc::Server> grpcServer = grpcBuilder.BuildAndStart();
        LOGI(LogModule::App, "GateNotify gRPC service started on {}", grpcAddress);

        // 初始化线程池管理器（所有任务线程池在此创建）
        ThreadPoolMgr::getInstance();

        // 7. 创建 IO 上下文并启动 HTTP 服务器
        net::io_context ioc{1};

        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait(
            [&ioc, &grpcServer](const boost::system::error_code &error, int signalNumber) {
                if (error)
                {
                    LOGE(LogModule::App, "Signal error: {}", error.message());
                    return;
                }
                LOGI(LogModule::App, "Received signal {}, stopping servers", signalNumber);
                grpcServer->Shutdown();
                ioc.stop();
            });

        auto getHandler = [logicSystem](const std::string &path,
                                        std::shared_ptr<HttpConnection> conn) {
            return logicSystem->handleGet(path, conn);
        };
        auto postHandler = [logicSystem](const std::string &path,
                                         std::shared_ptr<HttpConnection> conn) {
            return logicSystem->handlePost(path, conn);
        };
        std::make_shared<CServer>(ioc, gatePort, getHandler, postHandler)->start();
        LOGI(LogModule::App, "HTTP server started");

        // MySQL 健康检查 → 挂在 acceptor 的 io_context 上，用定时器替代独立线程
        MySqlPool::getInstance().startHealthCheck(ioc);

        // 主线程运行 acceptor io_context（阻塞直到收到信号）
        LOGI(LogModule::App, "Entering io_context run loop (main thread)");
        ioc.run();

        LOGI(LogModule::App, "GateServer stopped");
        Log::shutdown();
    }
    catch (std::exception &e)
    {
        std::cerr << "GateServer exception: " << e.what() << std::endl;
        LOGE(LogModule::App, "GateServer exception: {}", e.what());
        Log::shutdown();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
