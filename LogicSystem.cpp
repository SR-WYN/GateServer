#include "LogicSystem.h"
#include "HttpConnection.h"

LogicSystem::~LogicSystem()
{
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
    _get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
    _post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    RegGet("/get_test",
           [](std::shared_ptr<HttpConnection> connection)
           {
               beast::ostream(connection->GetResponse().body()) << "recvive get_test req";
               int i = 0;
               for (auto& elem : connection->GetParams())
               {
                   i++;
                   beast::ostream(connection->GetResponse().body())
                       << "param " << i << " key is " << elem.first << std::endl;
                   beast::ostream(connection->GetResponse().body())
                       << "param " << i << " value is " << elem.second << std::endl;
               }
           });
    RegPost("/get_verify_code",
            [](std::shared_ptr<HttpConnection> connection) -> bool
            {
                auto body_str =
                    boost::beast::buffers_to_string(connection->GetRequest().body().data());
                std::cout << "recvive body is " << body_str << std::endl;
                connection->GetResponse().set(http::field::content_type, "text/json");
                Json::Value root;
                Json::Reader reader;
                Json::Value src_root;
                bool parse_success = reader.parse(body_str, src_root);
                if (!parse_success || !src_root.isMember("email"))
                {
                    std::cout << "Failed to parse Json data" << std::endl;
                    root["error"] = ErrorCodes::ERROR_JSON;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return true;
                }
                auto email = src_root["email"].asString();
                std::cout << "email is " << email << std::endl;
                root["error"] = ErrorCodes::SUCCESS;
                root["email"] = src_root["email"];
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return true;
            });
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
    if (_get_handlers.find(path) == _get_handlers.end())
    {
        return false;
    }
    _get_handlers[path](con);
    return true;
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    if (_post_handlers.find(path) == _post_handlers.end())
    {
        return false;
    }
    _post_handlers[path](con);
    return true;
}