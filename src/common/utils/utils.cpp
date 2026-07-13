// utils.cpp - 通用工具集合实现
#include "utils.h"

namespace utils::url
{

static unsigned char toHex(unsigned char x)
{
    return x > 9 ? x + 55 : x + 48;
}

static unsigned char fromHex(unsigned char x)
{
    if (x >= 'A' && x <= 'F')
        return x - 'A' + 10;
    else if (x >= 'a' && x <= 'f')
        return x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        return x - '0';
    else
        return 0;
}

std::string encode(const std::string &str)
{
    std::string strTemp = "";
    for (unsigned char c : str)
    {
        if (std::isalnum(c) || (c == '-') || (c == '_') || (c == '.') || (c == '~'))
        {
            strTemp += c;
        }
        else if (c == ' ')
        {
            strTemp += "%20";
        }
        else
        {
            strTemp += '%';
            strTemp += toHex(c >> 4);
            strTemp += toHex(c & 0x0F);
        }
    }
    return strTemp;
}

std::string decode(const std::string &str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+')
        {
            strTemp += ' ';
        }
        else if (str[i] == '%' && i + 2 < length)
        {
            unsigned char high = fromHex(str[++i]);
            unsigned char low = fromHex(str[++i]);
            strTemp += static_cast<unsigned char>((high << 4) | low);
        }
        else
        {
            strTemp += str[i];
        }
    }
    return strTemp;
}

} // namespace utils::url

namespace utils::log
{

spdlog::level::level_enum parseLevel(const std::string &level_str)
{
    std::string level = level_str;
    std::transform(level.begin(), level.end(), level.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (level == "trace")
    {
        return spdlog::level::trace;
    }
    if (level == "debug")
    {
        return spdlog::level::debug;
    }
    if (level == "info")
    {
        return spdlog::level::info;
    }
    if (level == "warn" || level == "warning")
    {
        return spdlog::level::warn;
    }
    if (level == "error" || level == "err")
    {
        return spdlog::level::err;
    }
    if (level == "critical" || level == "fatal")
    {
        return spdlog::level::critical;
    }
    if (level == "off")
    {
        return spdlog::level::off;
    }
    return spdlog::level::info;
}

} // namespace utils::log
