// JsonUtil.cpp - JSON 解析与响应构建工具实现
#include "json_util.h"
#include "Log.h"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/ostream.hpp>
#include <json/reader.h>

Json::Value JsonUtil::parseRequestBody(std::shared_ptr<HttpConnection> conn)
{
    auto body_str = boost::beast::buffers_to_string(conn->getRequest().body().data());
    Json::Value root;
    Json::Reader reader;
    bool success = reader.parse(body_str, root);
    if (!success)
    {
        Log::error(LogModule::Http, "JSON parse error: {}", body_str);
        return Json::Value(); // 返回 null Value
    }
    return root;
}

void JsonUtil::makeJsonResponse(std::shared_ptr<HttpConnection> conn, const Json::Value &root)
{
    conn->getResponse().set(http::field::content_type, "text/json");
    std::string jsonstr = root.toStyledString();
    beast::ostream(conn->getResponse().body()) << jsonstr;
}

void JsonUtil::makeErrorResponse(std::shared_ptr<HttpConnection> conn, int errorCode)
{
    Json::Value root;
    root["error"] = errorCode;
    makeJsonResponse(conn, root);
}
