// HttpConnection.h - HTTP 连接处理类，负责异步读写与请求分发
#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

// HttpConnection 继承 enable_shared_from_this，通过 shared_ptr 管理异步生命周期
class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(
        boost::asio::io_context &ioc,
        std::function<bool(const std::string &, std::shared_ptr<HttpConnection>)> getHandler,
        std::function<bool(const std::string &, std::shared_ptr<HttpConnection>)> postHandler);
    // 启动异步读取 HTTP 请求
    void start();
    tcp::socket &getSocket();
    http::response<http::dynamic_body> &getResponse();
    http::request<http::dynamic_body> &getRequest();
    std::unordered_map<std::string, std::string> &getParams();

private:
    // 超时检测，60 秒无响应则关闭连接
    void checkDeadline();
    // 异步写入 HTTP 响应
    void writeResponse();
    // 根据 method 分发 GET/POST 请求
    void handleReq();
    // 解析 GET 请求的 URL 查询参数
    void preParseGetParam();

    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;
    net::steady_timer _deadline{_socket.get_executor(), std::chrono::seconds(60)};
    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params;
    std::function<bool(const std::string &, std::shared_ptr<HttpConnection>)> _getHandler;
    std::function<bool(const std::string &, std::shared_ptr<HttpConnection>)> _postHandler;

    std::chrono::steady_clock::time_point _req_start;
    std::string _peer_ip{"unknown"};
    unsigned short _peer_port{0};
};

using HttpGetHandler = std::function<bool(const std::string &, std::shared_ptr<HttpConnection>)>;
using HttpPostHandler = std::function<bool(const std::string &, std::shared_ptr<HttpConnection>)>;
