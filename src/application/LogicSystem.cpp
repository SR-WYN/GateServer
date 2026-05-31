// LogicSystem.cpp - 路由注册与分发，不再包含具体业务逻辑
#include "LogicSystem.h"
#include "HttpConnection.h"
#include "UserController.h"
#include "VerifyController.h"

#include <boost/beast/core/ostream.hpp>

LogicSystem::~LogicSystem()
{
}

void LogicSystem::regGet(std::string url, HttpHandler handler)
{
    _get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::regPost(std::string url, HttpHandler handler)
{
    _post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    regGet("/get_test",
           [](std::shared_ptr<HttpConnection> connection)
           {
               beast::ostream(connection->GetResponse().body()) << "recvive get_test req";
               int i = 0;
               for (auto& elem : connection->GetParams())
               {
                   i++;
                   beast::ostream(connection->GetResponse().body())
                       << "param " << i << " key is " << elem.first << std::endl;
                   beast::ostream(connection->GetResponse().body())
                       << "param " << i << " value is " << elem.second << std::endl;
               }
           });

    regPost("/get_verify_code", VerifyController::handleGetVerifyCode);
    regPost("/user_register",   UserController::handleRegister);
    regPost("/reset_pwd",       UserController::handleResetPwd);
    regPost("/user_login",      UserController::handleLogin);
}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
    auto it = _get_handlers.find(path);
    if (it == _get_handlers.end())
    {
        return false;
    }
    it->second(con);
    return true;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    auto it = _post_handlers.find(path);
    if (it == _post_handlers.end())
    {
        return false;
    }
    it->second(con);
    return true;
}
