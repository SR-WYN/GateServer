#pragma once

#include "Singleton.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

class HttpConnection;
class UserController;
class VerifyController;

typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    bool handleGet(std::string, std::shared_ptr<HttpConnection>);
    void regGet(std::string, HttpHandler);
    void regPost(std::string, HttpHandler);
    bool handlePost(std::string, std::shared_ptr<HttpConnection>);

private:
    LogicSystem();
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;

    // Controller 实例（通过 shared_ptr 管理生命周期）
    std::shared_ptr<UserController> _userController;
    std::shared_ptr<VerifyController> _verifyController;
};
