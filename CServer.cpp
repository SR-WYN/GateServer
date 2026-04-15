#include "CServer.h"
#include <iostream>
#include "HttpConnection.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port)
    : _ioc(ioc),
      _acceptor(ioc, tcp::endpoint(tcp::v4(), port)),
      _socket(ioc)
{
}

void CServer::Start()
{
    auto self = shared_from_this();
    _acceptor.async_accept(_socket,
                           [self](beast::error_code ec)
                           {
                               try
                               {
                                   // 出错放弃连接，继续监听其它连接
                                   if (ec)
                                   {
                                       self->Start();
                                       return;
                                   }

                                   // 创建新连接，并且创建HttpConnection管理连接
                                   std::make_shared<HttpConnection>(std::move(self->_socket))->Start();

                                   self->Start();
                               }
                               catch (const std::exception& e)
                               {
                                   std::cerr << "Exception: " << e.what() << std::endl;
                               }
                           });
}