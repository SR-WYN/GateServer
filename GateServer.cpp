#include "CServer.h"
#include <cstdlib>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include "ConfigMgr.h"
#include "LogicSystem.h"

void init();

int main()
{
    try
    {
        init();
        {
            auto& config = ConfigMgr::GetInstance();
            std::string gate_port_str = config["GateServer"]["Port"];
            unsigned short gate_port = static_cast<unsigned short>(std::stoi(gate_port_str));
            net::io_context ioc{1};
            boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait(
                [&ioc](const boost::system::error_code& error, int signal_number)
                {
                    if (error)
                    {
                        return;
                    }
                    ioc.stop();
                });
            std::make_shared<CServer>(ioc,gate_port)->Start();
            std::cout << "Gate Server listen on port: " << gate_port << std::endl;
            ioc.run();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

void init()
{
    ConfigMgr::GetInstance();
    LogicSystem::GetInstance();

    std::cout << "Gate Server init done" << std::endl;
}