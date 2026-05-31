// JsonUtil.h - JSON 解析与响应构建工具，消除 handler 中的重复代码
#pragma once

#include "HttpConnection.h"
#include <json/json.h>
#include <json/value.h>
#include <memory>

class JsonUtil
{
public:
    // 从 HTTP 请求体中解析 JSON，解析失败返回 null Value
    static Json::Value parseRequestBody(std::shared_ptr<HttpConnection> conn);

    // 设置 content-type 为 json，并将 root 序列化后写入响应体
    static void makeJsonResponse(std::shared_ptr<HttpConnection> conn, const Json::Value& root);

    // 快捷方法：构造并发送仅包含 error 字段的 JSON 错误响应
    static void makeErrorResponse(std::shared_ptr<HttpConnection> conn, int errorCode);
};
