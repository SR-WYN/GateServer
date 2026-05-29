#include "CServer.h"
#include "ConfigMgr.h"
#include "Log.h"
#include "LogicSystem.h"
#include "RedisMgr.h"
#include "http_types.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

void testRedis();
void init();
void testRedisMgr();

int main()
{
    try
    {
        init();
        if (!Log::init("GateServer", ConfigMgr::getInstance().getLogConfig()))
        {
            return EXIT_FAILURE;
        }
        Log::info(LogModule::App, "GateServer starting");
        // testRedisMgr();
        // testRedis();
        {
            auto& config = ConfigMgr::getInstance();
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
            std::make_shared<CServer>(ioc, gate_port)->start();
            ioc.run();
        }
        Log::info(LogModule::App, "GateServer stopped");
        Log::shutdown();
    }
    catch (std::exception& e)
    {
        Log::error(LogModule::App, "GateServer exception: {}", e.what());
        Log::shutdown();
        return EXIT_FAILURE;
    }
}

void init()
{
    ConfigMgr::getInstance();
    LogicSystem::getInstance();

}

void testRedis()
{
    // 使用 redis-plus-plus 替代 hiredis 测试
    sw::redis::ConnectionOptions conn_opts;
    conn_opts.host = "127.0.0.1";
    conn_opts.port = 6379;
    conn_opts.password = "123456";
    conn_opts.socket_timeout = std::chrono::milliseconds(3000);
    sw::redis::Redis redis(conn_opts);

    redis.set("stest1", "value1");
    assert(redis.strlen("stest1") == 6);

    auto val = redis.get("stest1");
    assert(val.has_value() && *val == "value1");

    auto nil_val = redis.get("stest2");
    assert(!nil_val.has_value());
}

void testRedisMgr()
{
    // redis-plus-plus 在构造函数中自动连接和认证，无需手动 connect/auth
    assert(RedisMgr::getInstance().set("blogwebsite", "llfc.club"));
    std::string value = "";
    assert(RedisMgr::getInstance().get("blogwebsite", value));
    assert(RedisMgr::getInstance().get("nonekey", value) == false);
    assert(RedisMgr::getInstance().hSet("bloginfo", "blogwebsite", "llfc.club"));
    assert(RedisMgr::getInstance().hGet("bloginfo", "blogwebsite") != "");
    assert(RedisMgr::getInstance().existsKey("bloginfo"));
    assert(RedisMgr::getInstance().del("bloginfo"));
    assert(RedisMgr::getInstance().del("bloginfo"));
    assert(RedisMgr::getInstance().existsKey("bloginfo") == false);
    assert(RedisMgr::getInstance().lPush("lpushkey1", "lpushvalue1"));
    assert(RedisMgr::getInstance().lPush("lpushkey1", "lpushvalue2"));
    assert(RedisMgr::getInstance().lPush("lpushkey1", "lpushvalue3"));
    assert(RedisMgr::getInstance().rPop("lpushkey1", value));
    assert(RedisMgr::getInstance().rPop("lpushkey1", value));
    assert(RedisMgr::getInstance().lPop("lpushkey1", value));
    assert(RedisMgr::getInstance().lPop("lpushkey2", value) == false);
    RedisMgr::getInstance().close();
}
