// test_utils.cpp - GateServer 纯函数工具类单元测试
#include <gtest/gtest.h>
#include "utils.h"
#include "defer.h"
#include <spdlog/common.h>

// ==================== utils::url ====================

class UrlEncodeTest : public ::testing::Test
{
};

TEST_F(UrlEncodeTest, PlainAscii)
{
    EXPECT_EQ(utils::url::encode("hello"), "hello");
}

TEST_F(UrlEncodeTest, Space)
{
    EXPECT_EQ(utils::url::encode("hello world"), "hello%20world");
}

TEST_F(UrlEncodeTest, ChineseChars)
{
    // "你好" UTF-8: E4 BD A0 E5 A5 BD
    std::string result = utils::url::encode("你好");
    EXPECT_EQ(result, "%E4%BD%A0%E5%A5%BD");
}

TEST_F(UrlEncodeTest, SpecialChars)
{
    EXPECT_EQ(utils::url::encode("a+b=c&d"), "a%2Bb%3Dc%26d");
}

TEST_F(UrlEncodeTest, ReservedChars)
{
    // - _ . ~ 应原样保留
    EXPECT_EQ(utils::url::encode("a-b_c.d~e"), "a-b_c.d~e");
}

TEST_F(UrlEncodeTest, Empty)
{
    EXPECT_EQ(utils::url::encode(""), "");
}

class UrlDecodeTest : public ::testing::Test
{
};

TEST_F(UrlDecodeTest, PlainAscii)
{
    EXPECT_EQ(utils::url::decode("hello"), "hello");
}

TEST_F(UrlDecodeTest, PercentEncoding)
{
    EXPECT_EQ(utils::url::decode("hello%20world"), "hello world");
}

TEST_F(UrlDecodeTest, PlusAsSpace)
{
    EXPECT_EQ(utils::url::decode("hello+world"), "hello world");
}

TEST_F(UrlDecodeTest, RoundTrip)
{
    std::string original = "hello world! 你好";
    EXPECT_EQ(utils::url::decode(utils::url::encode(original)), original);
}

TEST_F(UrlDecodeTest, Empty)
{
    EXPECT_EQ(utils::url::decode(""), "");
}

// ==================== utils::log::parseLevel ====================

class ParseLevelTest : public ::testing::Test
{
};

TEST_F(ParseLevelTest, Trace)
{
    EXPECT_EQ(utils::log::parseLevel("trace"), spdlog::level::trace);
}

TEST_F(ParseLevelTest, Debug)
{
    EXPECT_EQ(utils::log::parseLevel("debug"), spdlog::level::debug);
}

TEST_F(ParseLevelTest, Info)
{
    EXPECT_EQ(utils::log::parseLevel("info"), spdlog::level::info);
}

TEST_F(ParseLevelTest, Warn)
{
    EXPECT_EQ(utils::log::parseLevel("warn"), spdlog::level::warn);
}

TEST_F(ParseLevelTest, WarningAlias)
{
    EXPECT_EQ(utils::log::parseLevel("warning"), spdlog::level::warn);
}

TEST_F(ParseLevelTest, Error)
{
    EXPECT_EQ(utils::log::parseLevel("error"), spdlog::level::err);
}

TEST_F(ParseLevelTest, ErrAlias)
{
    EXPECT_EQ(utils::log::parseLevel("err"), spdlog::level::err);
}

TEST_F(ParseLevelTest, Critical)
{
    EXPECT_EQ(utils::log::parseLevel("critical"), spdlog::level::critical);
}

TEST_F(ParseLevelTest, FatalAlias)
{
    EXPECT_EQ(utils::log::parseLevel("fatal"), spdlog::level::critical);
}

TEST_F(ParseLevelTest, Off)
{
    EXPECT_EQ(utils::log::parseLevel("off"), spdlog::level::off);
}

TEST_F(ParseLevelTest, Uppercase)
{
    EXPECT_EQ(utils::log::parseLevel("INFO"), spdlog::level::info);
    EXPECT_EQ(utils::log::parseLevel("Warn"), spdlog::level::warn);
}

TEST_F(ParseLevelTest, UnknownDefaultsToInfo)
{
    EXPECT_EQ(utils::log::parseLevel("verbose"), spdlog::level::info);
    EXPECT_EQ(utils::log::parseLevel("anything"), spdlog::level::info);
}

// ==================== utils::Defer ====================

class DeferTest : public ::testing::Test
{
};

TEST_F(DeferTest, ExecutesOnScopeExit)
{
    bool called = false;
    {
        utils::Defer d([&called]() { called = true; });
        EXPECT_FALSE(called);
    }
    EXPECT_TRUE(called);
}

TEST_F(DeferTest, ExecutesOnlyOnce)
{
    int count = 0;
    {
        utils::Defer d([&count]() { count++; });
    }
    EXPECT_EQ(count, 1);
}

TEST_F(DeferTest, DoesNotExecuteIfNotExited)
{
    int count = 0;
    {
        utils::Defer d([&count]() { count++; });
        EXPECT_EQ(count, 0);
    }
    EXPECT_EQ(count, 1);
}
