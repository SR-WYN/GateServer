#pragma once
#include "Singleton.h"
#include "const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    bool handleGet(std::string,std::shared_ptr<HttpConnection>);
    void regGet(std::string,HttpHandler);
    void regPost(std::string,HttpHandler);
    bool handlePost(std::string,std::shared_ptr<HttpConnection>);

private:
    LogicSystem();
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;
};