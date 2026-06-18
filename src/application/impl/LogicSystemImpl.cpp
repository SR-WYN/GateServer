// LogicSystemImpl.cpp - HTTP 路由分发器实现
#include "LogicSystemImpl.h"

#include "HttpConnection.h"
#include "Log.h"
#include "LogModule.h"

LogicSystemImpl::LogicSystemImpl()
{
    LOGI(LogModule::App, "LogicSystemImpl created");
}

LogicSystemImpl::~LogicSystemImpl()
{
    LOGI(LogModule::App, "LogicSystemImpl destroyed");
}

void LogicSystemImpl::regGet(const std::string &path, HttpHandler handler)
{
    LOGI(LogModule::App, "Register GET route: {}", path);
    _getHandlers.emplace(path, std::move(handler));
}

void LogicSystemImpl::regPost(const std::string &path, HttpHandler handler)
{
    LOGI(LogModule::App, "Register POST route: {}", path);
    _postHandlers.emplace(path, std::move(handler));
}

bool LogicSystemImpl::handleGet(const std::string &path, std::shared_ptr<HttpConnection> conn)
{
    auto it = _getHandlers.find(path);
    if (it == _getHandlers.end())
    {
        LOGW(LogModule::Http, "GET route not found: {}", path);
        return false;
    }
    LOGI(LogModule::Http, "Dispatching GET: {}", path);
    it->second(conn);
    return true;
}

bool LogicSystemImpl::handlePost(const std::string &path, std::shared_ptr<HttpConnection> conn)
{
    auto it = _postHandlers.find(path);
    if (it == _postHandlers.end())
    {
        LOGW(LogModule::Http, "POST route not found: {}", path);
        return false;
    }
    LOGI(LogModule::Http, "Dispatching POST: {}", path);
    it->second(conn);
    return true;
}
