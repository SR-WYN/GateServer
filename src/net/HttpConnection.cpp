// HttpConnection.cpp - HTTP 连接处理实现，异步读写 + 请求分发
#include "HttpConnection.h"
#include "Log.h"
#include "LogicSystem.h"
#include "utils.h"

HttpConnection::HttpConnection(boost::asio::io_context &ioc) : _socket(ioc)
{
}

// 启动异步读取 HTTP 请求，读取完成后自动分发处理
void HttpConnection::start()
{
    auto self = shared_from_this();
    http::async_read(_socket, _buffer, _request,
                     [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                         try
                         {
                             if (ec)
                             {
                                 Log::error(LogModule::Http, "async_read error: {}", ec.message());
                                 return;
                             }
                             boost::ignore_unused(bytes_transferred);
                             Log::info(LogModule::Http, "Received request: {} {}",
                                       self->_request.method_string(), self->_request.target());
                             self->handleReq();
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
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos)
    {
        _get_url = uri;
        return;
    }

    _get_url = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos)
    {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos)
        {
            key = utils::urlDecode(pair.substr(0, eq_pos));
            value = utils::urlDecode(pair.substr(eq_pos + 1));
            _get_params[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）
    if (!query_string.empty())
    {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos)
        {
            key = utils::urlDecode(query_string.substr(0, eq_pos));
            value = utils::urlDecode(query_string.substr(eq_pos + 1));
            _get_params[key] = value;
        }
    }
}

// 根据 HTTP method 分发请求到 LogicSystem，处理 GET/POST
void HttpConnection::handleReq()
{
    // 设置版本
    _response.version(_request.version());
    _response.keep_alive(false);
    if (_request.method() == http::verb::get)
    {
        preParseGetParam();
        Log::info(LogModule::Http, "Handle GET: {}", _get_url);
        bool success = LogicSystem::getInstance().handleGet(_get_url, shared_from_this());
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
        Log::info(LogModule::Http, "Handle POST: {}", _request.target());
        bool success = LogicSystem::getInstance().handlePost(_request.target(), shared_from_this());
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
                          if (ec)
                          {
                              Log::error(LogModule::Http, "async_write error: {}", ec.message());
                          }
                          Log::info(LogModule::Http, "Response sent, closing connection");
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
            Log::warn(LogModule::Http, "Connection timeout, closing socket");
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
