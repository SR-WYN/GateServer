#include "utils.h"

unsigned char utils::ToHex(unsigned char x)
{
    return x > 9 ? x + 55 : x + 48;
}

unsigned char utils::FromHex(unsigned char x)
{
    if (x >= 'A' && x <= 'Z')
        return x - 'A' + 10;
    else if (x >= 'a' && x <= 'z')
        return x - 'a' + 10;
    else if (x >= '0' && x <= '9')
        return x - '0';
    else
        return 0;
}

std::string utils::UrlEncode(const std::string& str)
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
            strTemp += ToHex(c >> 4);   // 取高4位
            strTemp += ToHex(c & 0x0F); // 取低4位
        }
    }
    return strTemp;
}

std::string utils::UrlDecode(const std::string& str)
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
            unsigned char high = FromHex(str[++i]);
            unsigned char low = FromHex(str[++i]);
            strTemp += static_cast<unsigned char>((high << 4) | low);
        }
        else
        {
            strTemp += str[i];
        }
    }
    return strTemp;
}

utils::Defer::Defer(std::function<void()> func)
    : _func(func)
{
}

utils::Defer::~Defer()
{
    if (_func)
        _func();
}