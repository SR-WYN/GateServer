// GateServer.cpp - 网关服务器入口
#include "CServer.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "LogicSystem.h"
#include "RedisMgr.h"
#include "HttpConnection.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

void init();

int main()
{
    try
    {
        // 初始化配置与逻辑系统
        init();

        // 初始化日志模块（GateServer 日志）
        if (!Log::init("GateServer", ConfigMgr::getInstance().getLogConfig()))
        {
            std::cerr << "Log init failed" << std::endl;
            return EXIT_FAILURE;
        }
        Log::info(LogModule::App, "GateServer starting");

        {
            // 读取配置中的网关端口
            auto& config = ConfigMgr::getInstance();
            std::string gate_port_str = config["GateServer"]["Port"];
            unsigned short gate_port = static_cast<unsigned short>(std::stoi(gate_port_str));
            Log::info(LogModule::App, "GateServer port: {}", gate_port);

            // 创建 IO 上下文（仅 1 个线程）
            net::io_context ioc{1};
            // 注册 SIGINT/SIGTERM 信号处理，优雅关闭
            boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait(
                [&ioc](const boost::system::error_code& error, int signal_number)
                {
                    if (error)
                    {
                        Log::error(LogModule::App, "Signal error: {}", error.message());
                        return;
                    }
                    Log::info(LogModule::App, "Received signal {}, stopping io_context", signal_number);
                    ioc.stop();
                });

            // 启动 HTTP 服务器（CServer 继承 enable_shared_from_this，异步回调中需捕获自身 shared_ptr）
            std::make_shared<CServer>(ioc, gate_port)->start();
            Log::info(LogModule::App, "CServer started, entering io_context run loop");
            // 进入事件循环（阻塞，直到 ioc.stop()）
            ioc.run();
        }

        Log::info(LogModule::App, "GateServer stopped");
        Log::shutdown();
    }
    catch (std::exception& e)
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
