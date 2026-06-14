// GateServer.cpp - 网关服务器入口
#include "CServer.h"
#include "ConfigMgr.h"
#include "HttpConnection.h"
#include "Log.h"
#include "LogicSystem.h"
#include "RedisMgr.h"

#include "GateService.h"

#include <grpcpp/grpcpp.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <thread>

void init();

int main()
{
    try
    {
        // 初始化配置与逻辑系统
        init();

        Log::info(LogModule::App, "GateServer starting");

        {
            // 读取配置中的网关端口
            auto &config = ConfigMgr::getInstance();
            std::string gate_port_str = config["GateServer"]["Port"];
            unsigned short gate_port = static_cast<unsigned short>(std::stoi(gate_port_str));
            Log::info(LogModule::App, "GateServer HTTP port: {}", gate_port);

            // 读取 gRPC 服务端口
            std::string grpc_port_str = config["GateServer"]["GrpcPort"];
            if (grpc_port_str.empty()) {
                grpc_port_str = "51052"; // 默认 gRPC 端口
            }
            std::string grpc_address = "0.0.0.0:" + grpc_port_str;
            Log::info(LogModule::App, "GateServer gRPC address: {}", grpc_address);

            // 创建 gRPC 服务
            auto user_controller = LogicSystem::getInstance().getUserController();
            GateService gate_service(user_controller);
            grpc::ServerBuilder grpc_builder;
            grpc_builder.AddListeningPort(grpc_address, grpc::InsecureServerCredentials());
            grpc_builder.RegisterService(&gate_service);
            std::unique_ptr<grpc::Server> grpc_server = grpc_builder.BuildAndStart();
            Log::info(LogModule::App, "GateNotify gRPC service started on {}", grpc_address);

            // 在独立线程中运行 gRPC 服务
            std::thread grpc_thread([&grpc_server]() {
                Log::info(LogModule::App, "gRPC server thread running");
                grpc_server->Wait();
                Log::info(LogModule::App, "gRPC server thread stopped");
            });

            // 创建 IO 上下文（仅 1 个线程）
            net::io_context ioc{1};
            // 注册 SIGINT/SIGTERM 信号处理，优雅关闭
            boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc, &grpc_server](const boost::system::error_code &error, int signal_number) {
                if (error)
                {
                    Log::error(LogModule::App, "Signal error: {}", error.message());
                    return;
                }
                Log::info(LogModule::App, "Received signal {}, stopping servers", signal_number);
                grpc_server->Shutdown();
                ioc.stop();
            });

            // 启动 HTTP 服务器
            std::make_shared<CServer>(ioc, gate_port)->start();
            Log::info(LogModule::App, "HTTP server started, entering io_context run loop");
            ioc.run();

            // 等待 gRPC 线程结束
            if (grpc_thread.joinable()) {
                grpc_thread.join();
            }
        }

        Log::info(LogModule::App, "GateServer stopped");
        Log::shutdown();
    }
    catch (std::exception &e)
    {
        std::cerr << "GateServer exception: " << e.what() << std::endl;
        Log::error(LogModule::App, "GateServer exception: {}", e.what());
        Log::shutdown();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// 初始化全局单例：配置管理器与逻辑系统
void init()
{
    ConfigMgr::getInstance();
    LogicSystem::getInstance();
}
