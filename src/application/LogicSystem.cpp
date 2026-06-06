// LogicSystem.cpp - 路由注册与分发
// 负责依赖组装：创建适配器 → 创建 Controller → 注册路由
#include "LogicSystem.h"
#include "HttpConnection.h"
#include "Log.h"
#include "UserController.h"
#include "VerifyController.h"

// 接口定义
#include "IUserDao.h"
#include "IVerifyCodeCache.h"
#include "IUserCache.h"
#include "IStatusRpcClient.h"
#include "IVerifyRpcClient.h"

// 适配器实现
#include "UserDaoAdapter.h"
#include "VerifyCodeCacheAdapter.h"
#include "UserCacheAdapter.h"
#include "StatusRpcClientAdapter.h"
#include "VerifyRpcClientAdapter.h"

#include <boost/beast/core/ostream.hpp>

LogicSystem::~LogicSystem()
{
    Log::info(LogModule::App, "LogicSystem destroyed");
}

void LogicSystem::regGet(std::string url, HttpHandler handler)
{
    Log::info(LogModule::App, "Register GET route: {}", url);
    _get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::regPost(std::string url, HttpHandler handler)
{
    Log::info(LogModule::App, "Register POST route: {}", url);
    _post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    Log::info(LogModule::App, "LogicSystem initializing, registering routes...");

    // ===== 依赖组装（依赖注入容器） =====

    // 1. 创建基础设施适配器
    auto userDao      = std::make_shared<UserDaoAdapter>();
    auto verifyCache  = std::make_shared<VerifyCodeCacheAdapter>();
    auto userCache    = std::make_shared<UserCacheAdapter>();
    auto statusRpc    = std::make_shared<StatusRpcClientAdapter>();
    auto verifyRpc    = std::make_shared<VerifyRpcClientAdapter>();

    // 2. 创建 Controller 并注入依赖
    _userController = std::make_shared<UserController>(
        userDao, verifyCache, userCache, statusRpc);
    _verifyController = std::make_shared<VerifyController>(verifyRpc);

    // ===== 注册路由 =====

    regGet("/get_test",
           [](std::shared_ptr<HttpConnection> connection)
           {
               Log::info(LogModule::Http, "Handle GET /get_test");
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

    // 注册业务路由 — 绑定 Controller 实例方法
    regPost("/get_verify_code",
            [this](auto conn) { _verifyController->handleGetVerifyCode(conn); });
    regPost("/user_register",
            [this](auto conn) { _userController->handleRegister(conn); });
    regPost("/reset_pwd",
            [this](auto conn) { _userController->handleResetPwd(conn); });
    regPost("/user_login",
            [this](auto conn) { _userController->handleLogin(conn); });

    Log::info(LogModule::App, "LogicSystem initialized, {} GET routes, {} POST routes",
              _get_handlers.size(), _post_handlers.size());
}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
    auto it = _get_handlers.find(path);
    if (it == _get_handlers.end())
    {
        Log::warn(LogModule::Http, "GET route not found: {}", path);
        return false;
    }
    Log::info(LogModule::Http, "Dispatching GET: {}", path);
    it->second(con);
    return true;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    auto it = _post_handlers.find(path);
    if (it == _post_handlers.end())
    {
        Log::warn(LogModule::Http, "POST route not found: {}", path);
        return false;
    }
    Log::info(LogModule::Http, "Dispatching POST: {}", path);
    it->second(con);
    return true;
}
