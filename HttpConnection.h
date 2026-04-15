#pragma once

#include "const.h"
#include <boost/beast/http/dynamic_body.hpp>
#include <string>
#include <unordered_map>

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    HttpConnection(tcp::socket socket);
    void Start();
    http::response<http::dynamic_body>& GetResponse();
    http::request<http::dynamic_body>& GetRequest();
    std::unordered_map<std::string,std::string>& GetParams();
private:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();
    void PreParseGetParam();
    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;
    net::steady_timer _deadline{_socket.get_executor(), std::chrono::seconds(60)};
    std::string _get_url;
    std::unordered_map<std::string, std::string> _get_params; 
};