#pragma once

#include <string_view>

enum class LogModule
{
    App,
    Config,
    Http,
    Mysql,
    Redis,
    Grpc,
    Logic,
};

inline std::string_view moduleName(LogModule module)
{
    switch (module)
    {
    case LogModule::App:
        return "app";
    case LogModule::Config:
        return "config";
    case LogModule::Http:
        return "http";
    case LogModule::Mysql:
        return "mysql";
    case LogModule::Redis:
        return "redis";
    case LogModule::Grpc:
        return "grpc";
    case LogModule::Logic:
        return "logic";
    }
    return "unknown";
}
