// CServer.cpp - HTTP 服务器入口
#include "CServer.h"

#include "HttpConnection.h"
#include "Log.h"
#include "LogModule.h"
#include "ThreadPoolMgr.h"

CServer::CServer(boost::asio::io_context &ioc, unsigned short &port, HttpGetHandler getHandler,
                 HttpPostHandler postHandler)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port)), _getHandler(std::move(getHandler)),
      _postHandler(std::move(postHandler))
{
    Log::info(LogModule::App, "CServer constructed, listening on port {}", port);
}

void CServer::start()
{
    auto self = shared_from_this();
    auto &ioContext = ThreadPoolMgr::getInstance().getIoService();
    auto newCon = std::make_shared<HttpConnection>(ioContext, _getHandler, _postHandler);
    _acceptor.async_accept(newCon->getSocket(), [self, newCon](beast::error_code ec) {
        try
        {
            // 出错放弃连接，继续监听其它连接
            if (ec)
            {
                Log::error(LogModule::App, "CServer async_accept error: {}", ec.message());
                self->start();
                return;
            }
            // 创建新连接，并且创建 HttpConnection 管理连接
            Log::info(LogModule::App, "New connection accepted");
            newCon->start();
            self->start();
        }
        catch (const std::exception &e)
        {
            Log::error(LogModule::App, "CServer exception: {}", e.what());
        }
    });
}
