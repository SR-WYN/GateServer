#include "CServer.h"
#include "ConfigMgr.h"
#include "LogicSystem.h"
#include "RedisMgr.h"
#include "http_types.h"

#include <cassert>
#include <cstdlib>
#include <hiredis/hiredis.h>
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
    }
    catch (std::exception& e)
    {
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
    // 连接redis 需要启动才可以进行连接
    redisContext* c = redisConnect("127.0.0.1", 6379);
    if (c->err)
    {
        redisFree(c);
        return;
    }
    std::string redis_password = "123456";
    redisReply* r = (redisReply*)redisCommand(c, "AUTH %s", redis_password.c_str());
    if (r->type == REDIS_REPLY_ERROR)
    {
    }
    else
    {
    }
    // 为redis设置key
    const char* command1 = "set stest1 value1";
    // 执行redis命令行
    r = (redisReply*)redisCommand(c, command1);
    // 如果返回NULL则说明执行失败
    if (NULL == r)
    {
        redisFree(c);
        return;
    }
    // 如果执行失败则释放连接
    if (!(r->type == REDIS_REPLY_STATUS &&
          (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
    {
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(r);
    const char* command2 = "strlen stest1";
    r = (redisReply*)redisCommand(c, command2);
    // 如果返回类型不是整形 则释放连接
    if (r->type != REDIS_REPLY_INTEGER)
    {
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    // 获取字符串长度
    int length = r->integer;
    freeReplyObject(r);
    // 获取redis键值对信息
    const char* command3 = "get stest1";
    r = (redisReply*)redisCommand(c, command3);
    if (r->type != REDIS_REPLY_STRING)
    {
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    freeReplyObject(r);
    const char* command4 = "get stest2";
    r = (redisReply*)redisCommand(c, command4);
    if (r->type != REDIS_REPLY_NIL)
    {
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    freeReplyObject(r);
    // 释放连接资源
    redisFree(c);
}

void testRedisMgr()
{
    assert(RedisMgr::getInstance().connect("127.0.0.1", 6379));
    assert(RedisMgr::getInstance().auth("123456"));
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
