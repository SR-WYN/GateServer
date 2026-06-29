// utils.h - 通用工具集合
#pragma once
#include <string>

namespace utils::url
{

std::string encode(const std::string &str);
std::string decode(const std::string &str);

} // namespace utils::url