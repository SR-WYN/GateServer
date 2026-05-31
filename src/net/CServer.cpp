#include "AsioIOServicePool.h"
#include "CServer.h"
#include "HttpConnection.h"
#include "Log.h"
#include <iostream>

CServer::CServer(boost::asio::io_context &ioc, unsigned short &port)
    : _ioc(ioc), _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
    Log::info(LogModule::App, "CServer constructed, listening on port {}", port);
}

void CServer::start()
{
    auto self = shared_from_this();
    auto &io_context = AsioIOServicePool::getInstance().getIoService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    _acceptor.async_accept(new_con->GetSocket(), [self, new_con](beast::error_code ec) {
        try
        {
            // 出错放弃连接，继续监听其它连接
            if (ec)
            {
                Log::error(LogModule::App, "CServer async_accept error: {}", ec.message());
                self->start();
                return;
            }
            // 创建新连接，并且创建HttpConnection管理连接
            Log::info(LogModule::App, "New connection accepted");
            new_con->start();
            self->start();
        }
        catch (const std::exception &e)
        {
            Log::error(LogModule::App, "CServer exception: {}", e.what());
        }
    });
}
