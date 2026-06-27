// HttpConnection.cpp - HTTP 连接处理实现，异步读写 + 请求分发
#include "HttpConnection.h"

#include "Log.h"
#include "LogModule.h"
#include "ThreadPoolMgr.h"
#include "utils.h"

#include <chrono>

HttpConnection::HttpConnection(boost::asio::io_context &ioc, HttpGetHandler getHandler,
                               HttpPostHandler postHandler)
    : _socket(ioc), _getHandler(std::move(getHandler)), _postHandler(std::move(postHandler))
{
}

// 启动异步读取 HTTP 请求，读取完成后自动分发处理
void HttpConnection::start()
{
    auto self = shared_from_this();
    self->_req_start = std::chrono::steady_clock::now();

    try
    {
        self->_peer_ip = self->_socket.remote_endpoint().address().to_string();
        self->_peer_port = self->_socket.remote_endpoint().port();
    }
    catch (...)
    {
        self->_peer_ip = "unknown";
        self->_peer_port = 0;
    }

    http::async_read(_socket, _buffer, _request,
                     [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                         try
                         {
                             if (ec)
                             {
                                 Log::error(LogModule::Http,
                                            "async_read error peer={}:{} err={}", self->_peer_ip,
                                            self->_peer_port, ec.message());
                                 return;
                             }
                             boost::ignore_unused(bytes_transferred);
                             Log::info(LogModule::Http,
                                       "Received request peer={}:{} method={} target={}",
                                       self->_peer_ip, self->_peer_port,
                                       self->_request.method_string(), self->_request.target());
                             // 将业务处理投递到 HTTP 业务线程池，不阻塞 IO 线程
                             ThreadPoolMgr::getInstance().enqueueHttpLogic([self]() {
                                 self->handleReq();
                             });
                             self->checkDeadline();
                         }
                         catch (std::exception &e)
                         {
                             Log::error(LogModule::Http, "http read handler exception: {}",
                                        e.what());
                         }
                     });
}

// 解析 GET 请求的 URL 查询参数（如 ?key1=val1&key2=val2）
void HttpConnection::preParseGetParam()
{
    // 提取 URI
    auto uri = _request.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）
    auto queryPos = uri.find('?');
    if (queryPos == std::string::npos)
    {
        _get_url = uri;
        return;
    }

    _get_url = uri.substr(0, queryPos);
    std::string queryString = uri.substr(queryPos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = queryString.find('&')) != std::string::npos)
    {
        auto pair = queryString.substr(0, pos);
        size_t eqPos = pair.find('=');
        if (eqPos != std::string::npos)
        {
            key = utils::urlDecode(pair.substr(0, eqPos));
            value = utils::urlDecode(pair.substr(eqPos + 1));
            _get_params[key] = value;
        }
        queryString.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）
    if (!queryString.empty())
    {
        size_t eqPos = queryString.find('=');
        if (eqPos != std::string::npos)
        {
            key = utils::urlDecode(queryString.substr(0, eqPos));
            value = utils::urlDecode(queryString.substr(eqPos + 1));
            _get_params[key] = value;
        }
    }
}

// 根据 HTTP method 分发 GET/POST 请求
void HttpConnection::handleReq()
{
    // 设置版本
    _response.version(_request.version());
    _response.keep_alive(false);
    if (_request.method() == http::verb::get)
    {
        preParseGetParam();
        Log::info(LogModule::Http, "Handle GET: {} peer={}:{}", _get_url, _peer_ip, _peer_port);
        bool success = _getHandler(_get_url, shared_from_this());
        if (!success)
        {
            Log::warn(LogModule::Http, "GET url not found: {}", _get_url);
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "test/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            writeResponse();
            return;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");
        writeResponse();
        return;
    }
    if (_request.method() == http::verb::post)
    {
        Log::info(LogModule::Http, "Handle POST: {} peer={}:{}", _request.target(), _peer_ip,
                  _peer_port);
        bool success = _postHandler(_request.target(), shared_from_this());
        if (!success)
        {
            Log::warn(LogModule::Http, "POST url not found: {}", _request.target());
            _response.result(http::status::not_found);
            _response.set(http::field::content_type, "test/plain");
            beast::ostream(_response.body()) << "url not found\r\n";
            writeResponse();
            return;
        }

        _response.result(http::status::ok);
        _response.set(http::field::server, "GateServer");
        writeResponse();
        return;
    }
}

// 异步写入 HTTP 响应，写完后关闭连接发送端
void HttpConnection::writeResponse()
{
    auto self = shared_from_this();
    _response.content_length(_response.body().size());
    http::async_write(_socket, _response,
                      [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                          const auto cost_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                   std::chrono::steady_clock::now() -
                                                   self->_req_start)
                                                   .count();
                          if (ec)
                          {
                              Log::error(LogModule::Http,
                                         "async_write error peer={}:{} err={} status={}",
                                         self->_peer_ip, self->_peer_port, ec.message(),
                                         static_cast<int>(self->_response.result()));
                          }
                          else
                          {
                              Log::info(LogModule::Http,
                                        "Response sent peer={}:{} status={} bytes={} cost={}ms",
                                        self->_peer_ip, self->_peer_port,
                                        static_cast<int>(self->_response.result()),
                                        bytes_transferred, cost_ms);
                          }
                          self->_socket.shutdown(tcp::socket::shutdown_send, ec);
                          self->_deadline.cancel();
                      });
}

// 超时检测，60 秒无响应则关闭连接
void HttpConnection::checkDeadline()
{
    auto self = shared_from_this();
    _deadline.async_wait([self](beast::error_code ec) {
        if (!ec)
        {
            Log::warn(LogModule::Http, "Connection timeout peer={}:{} target={}", self->_peer_ip,
                      self->_peer_port, self->_request.target());
            self->_socket.close(ec);
        }
    });
}

http::response<http::dynamic_body> &HttpConnection::getResponse()
{
    return _response;
}

http::request<http::dynamic_body> &HttpConnection::getRequest()
{
    return _request;
}

std::unordered_map<std::string, std::string> &HttpConnection::getParams()
{
    return _get_params;
}

tcp::socket &HttpConnection::getSocket()
{
    return _socket;
}
