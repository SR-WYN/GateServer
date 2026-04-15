#pragma once
#include <string>

namespace utils
{

// 将字节转换为十六进制字符
unsigned char ToHex(unsigned char x);

// 将十六进制字符转换为字节
unsigned char FromHex(unsigned char x);

// 对URL进行编码
std::string UrlEncode(const std::string& str);

// 对URL进行解码
std::string UrlDecode(const std::string& str);

} // namespace utils