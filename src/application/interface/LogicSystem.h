// LogicSystem.h - HTTP 路由分发器接口
#pragma once

#include <functional>
#include <memory>
#include <string>

class HttpConnection;

using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem
{
public:
    virtual ~LogicSystem() = default;

    virtual void regGet(const std::string &path, HttpHandler handler) = 0;
    virtual void regPost(const std::string &path, HttpHandler handler) = 0;
    virtual bool handleGet(const std::string &path, std::shared_ptr<HttpConnection> conn) = 0;
    virtual bool handlePost(const std::string &path, std::shared_ptr<HttpConnection> conn) = 0;
};
