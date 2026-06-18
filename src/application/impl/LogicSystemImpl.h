// LogicSystemImpl.h - HTTP 路由分发器实现
#pragma once

#include "LogicSystem.h"
#include <map>

class LogicSystemImpl final : public LogicSystem
{
public:
    LogicSystemImpl();
    ~LogicSystemImpl() override;

    void regGet(const std::string &path, HttpHandler handler) override;
    void regPost(const std::string &path, HttpHandler handler) override;
    bool handleGet(const std::string &path, std::shared_ptr<HttpConnection> conn) override;
    bool handlePost(const std::string &path, std::shared_ptr<HttpConnection> conn) override;

private:
    std::map<std::string, HttpHandler> _getHandlers;
    std::map<std::string, HttpHandler> _postHandlers;
};
