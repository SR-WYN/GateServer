// CServer.h - HTTP 服务器入口
#pragma once

#include "HttpConnection.h"

#include <boost/asio.hpp>
#include <memory>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class CServer : public std::enable_shared_from_this<CServer>
{
public:
    CServer(boost::asio::io_context &ioc, unsigned short &port, HttpGetHandler getHandler,
            HttpPostHandler postHandler);
    void start();

private:
    tcp::acceptor _acceptor;
    net::io_context &_ioc;
    HttpGetHandler _getHandler;
    HttpPostHandler _postHandler;
};
