// test_logic_system.cpp - GateServer LogicSystemImpl 路由基本场景测试
#include <gtest/gtest.h>
#include "LogicSystemImpl.h"
#include "HttpConnection.h"

#include <boost/asio.hpp>

class GateServerLogicSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        logic_ = std::make_unique<LogicSystemImpl>();
    }

    std::unique_ptr<LogicSystemImpl> logic_;
    boost::asio::io_context ioc_;

    static auto makeEmptyHandler()
    {
        return [](const std::string &, std::shared_ptr<HttpConnection>) { return false; };
    }
};

TEST_F(GateServerLogicSystemTest, GetExactMatch)
{
    bool called = false;
    logic_->regGet("/test_get", [&called](std::shared_ptr<HttpConnection>) { called = true; });

    auto conn = std::make_shared<HttpConnection>(ioc_, makeEmptyHandler(), makeEmptyHandler());
    EXPECT_TRUE(logic_->handleGet("/test_get", conn));
    EXPECT_TRUE(called);
}

TEST_F(GateServerLogicSystemTest, PostExactMatch)
{
    bool called = false;
    logic_->regPost("/test_post", [&called](std::shared_ptr<HttpConnection>) { called = true; });

    auto conn = std::make_shared<HttpConnection>(ioc_, makeEmptyHandler(), makeEmptyHandler());
    EXPECT_TRUE(logic_->handlePost("/test_post", conn));
    EXPECT_TRUE(called);
}

TEST_F(GateServerLogicSystemTest, GetNotFound)
{
    auto conn = std::make_shared<HttpConnection>(ioc_, makeEmptyHandler(), makeEmptyHandler());
    EXPECT_FALSE(logic_->handleGet("/missing", conn));
}

TEST_F(GateServerLogicSystemTest, PostNotFound)
{
    auto conn = std::make_shared<HttpConnection>(ioc_, makeEmptyHandler(), makeEmptyHandler());
    EXPECT_FALSE(logic_->handlePost("/missing", conn));
}
