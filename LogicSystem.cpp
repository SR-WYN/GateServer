#include "LogicSystem.h"
#include "HttpConnection.h"
#include "RedisMgr.h"
#include "VerifyGrpcClient.h"
#include "const.h"

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
                GetVerifyRsp rsp = VerifyGrpcClient::GetInstance().GetVerifyCode(email);
                std::cout << "email is " << email << std::endl;
                root["error"] = rsp.error();
                root["email"] = src_root["email"];
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return true;
            });
    RegPost("/user_register",
            [](std::shared_ptr<HttpConnection> connection)
            {
                auto body_str =
                    boost::beast::buffers_to_string(connection->GetRequest().body().data());
                std::cout << "receive body is " << body_str << std::endl;
                connection->GetResponse().set(http::field::content_type, "text/json");
                Json::Value root;
                Json::Reader reader;
                Json::Value src_root;
                bool parse_success = reader.parse(body_str, src_root);
                if (!parse_success || !src_root.isMember("email") || !src_root.isMember("verify_code"))
                {
                    std::cout << "Failed to parse JSON data or missing fields! " << std::endl;
                    root["error"] = ErrorCodes::ERROR_JSON;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return true;
                }
                std::string redis_key = std::string(RedisPrefix::CODE) + src_root["email"].asString();
                std::string verify_code;
                bool b_get_verify =
                    RedisMgr::GetInstance().Get(redis_key, verify_code);
                if (!b_get_verify)
                {
                    std::cout << "get verify code expired" << std::endl;
                    root["error"] = ErrorCodes::VERIFY_EXPIRED;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return true;
                }
                if (verify_code != src_root["verify_code"].asString())
                {
                    std::cout << "verify code error" << std::endl;
                    root["error"] = ErrorCodes::VERIFY_CODE_ERROR;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return true;
                }
                bool b_user_exist = RedisMgr::GetInstance().ExistsKey(src_root["user"].asString());
                if (b_user_exist)
                {
                    std::cout << "user exist" << std::endl;
                    root["error"] = ErrorCodes::USER_EXIST;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return true;
                }
                auto passwd = src_root["passwd"].asString();
                auto confirm = src_root["confirm"].asString();
                if (passwd != confirm)
                {
                    std::cout << "passwd not match" << std::endl;
                    root["error"] = ErrorCodes::PASSWD_NOT_MATCH;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return true;
                }
                root["error"] = ErrorCodes::SUCCESS;
                root["email"] = src_root["email"];
                root["user"] = src_root["user"].asString();
                root["passwd"] = src_root["passwd"].asString();
                root["confirm"] = src_root["confirm"].asString();
                root["verify_code"] = src_root["verify_code"].asString();
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