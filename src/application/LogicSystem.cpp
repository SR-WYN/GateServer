#include "LogicSystem.h"
#include "HttpConnection.h"
#include "MysqlMgr.h"
#include "RedisMgr.h"
#include "StatusGrpcClient.h"
#include "VerifyGrpcClient.h"
#include "data.h"
#include "error_codes.h"
#include "http_types.h"

#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/ostream.hpp>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>

LogicSystem::~LogicSystem()
{
}

void LogicSystem::regGet(std::string url, HttpHandler handler)
{
    _get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::regPost(std::string url, HttpHandler handler)
{
    _post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem()
{
    regGet("/get_test",
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
    regPost("/get_verify_code",
            [](std::shared_ptr<HttpConnection> connection)
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
                    return;
                }
                auto email = src_root["email"].asString();
                GetVerifyRsp rsp = VerifyGrpcClient::getInstance().getVerifyCode(email);
                std::cout << "email is " << email << std::endl;
                root["error"] = rsp.error();
                root["email"] = src_root["email"];
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
            });
    regPost(
        "/user_register",
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
                return;
            }
            std::string redis_key = std::string(RedisPrefix::CODE) + src_root["email"].asString();
            std::string verify_code;
            bool b_get_verify = RedisMgr::getInstance().get(redis_key, verify_code);
            if (!b_get_verify)
            {
                std::cout << "get verify code expired" << std::endl;
                root["error"] = ErrorCodes::VERIFY_EXPIRED;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }
            if (verify_code != src_root["verify_code"].asString())
            {
                std::cout << "verify code error" << std::endl;
                root["error"] = ErrorCodes::VERIFY_CODE_ERROR;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }
            auto name = src_root["user"].asString();
            auto email = src_root["email"].asString();

            if (RedisMgr::getInstance().existsKey(name))
            {
                std::cout << "user exist in redis" << std::endl;
                root["error"] = ErrorCodes::USER_EXIST;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }
            if (MysqlMgr::getInstance().dao().userNameExists(name))
            {
                std::cout << "user exist in mysql" << std::endl;
                root["error"] = ErrorCodes::USER_EXIST;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }
            if (MysqlMgr::getInstance().dao().emailExists(email))
            {
                std::cout << "email exist in mysql" << std::endl;
                root["error"] = ErrorCodes::USER_EXIST;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }

            auto passwd = src_root["passwd"].asString();
            auto confirm = src_root["confirm"].asString();
            if (passwd != confirm)
            {
                std::cout << "passwd not match" << std::endl;
                root["error"] = ErrorCodes::PASSWD_NOT_MATCH;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }

            int uid = MysqlMgr::getInstance().dao().regUser(name, email, passwd);
            if (uid == 0)
            {
                std::cout << "user or email exist" << std::endl;
                root["error"] = ErrorCodes::USER_EXIST;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }
            if (uid < 0)
            {
                std::cout << "register database error" << std::endl;
                root["error"] = ErrorCodes::RPCFAILED;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
                return;
            }

            RedisMgr::getInstance().set(name, email);
            root["error"] = ErrorCodes::SUCCESS;
            root["email"] = email;
            root["user"] = name;
            root["passwd"] = passwd;
            root["confirm"] = confirm;
            root["verify_code"] = src_root["verify_code"].asString();
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->GetResponse().body()) << jsonstr;
        });
    regPost("/reset_pwd",
            [](std::shared_ptr<HttpConnection> connection)
            {
                auto body_str =
                    boost::beast::buffers_to_string(connection->GetRequest().body().data());
                std::cout << "recvive body is " << body_str << std::endl;
                connection->GetResponse().set(http::field::content_type, "text/json");
                Json::Value root;
                Json::Reader reader;
                Json::Value src_root;
                bool parse_success = reader.parse(body_str, src_root);
                if (!parse_success)
                {
                    std::cout << "Failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::ERROR_JSON;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                auto email = src_root["email"].asString();
                auto name = src_root["user"].asString();
                auto pwd = src_root["passwd"].asString();

                std::string verify_code;
                bool b_get_verify = RedisMgr::getInstance().get(
                    std::string(RedisPrefix::CODE) + email, verify_code);
                if (!b_get_verify)
                {
                    std::cout << "get verify code expired" << std::endl;
                    root["error"] = ErrorCodes::VERIFY_EXPIRED;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }
                if (verify_code != src_root["verify_code"].asString())
                {
                    std::cout << "verify code error" << std::endl;
                    root["error"] = ErrorCodes::VERIFY_CODE_ERROR;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                bool email_vaild = MysqlMgr::getInstance().dao().checkEmail(email, name);
                if (!email_vaild)
                {
                    std::cout << "email or user not exist" << std::endl;
                    root["error"] = ErrorCodes::EMAIL_NOT_MATCH;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                bool b_up = MysqlMgr::getInstance().dao().updatePwd(email, pwd);
                if (!b_up)
                {
                    std::cout << "update pwd failed" << std::endl;
                    root["error"] = ErrorCodes::PASSWD_UP_FAILED;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                std::cout << "succeed to update password" << pwd << std::endl;
                root["error"] = ErrorCodes::SUCCESS;
                root["email"] = email;
                root["user"] = name;
                root["passwd"] = pwd;
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
            });
    regPost("/user_login",
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
                if (!parse_success)
                {
                    std::cout << "Failed to parse JSON data! " << std::endl;
                    root["error"] = ErrorCodes::ERROR_JSON;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                auto email = src_root["email"].asString();
                auto pwd = src_root["passwd"].asString();
                UserInfo userInfo;
                bool pwd_vaild = MysqlMgr::getInstance().dao().checkPwd(email, pwd, userInfo);
                if (!pwd_vaild)
                {
                    std::cout << "email pwd not match " << std::endl;
                    root["error"] = ErrorCodes::PASSWD_NOT_MATCH;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                auto reply = StatusGrpcClient::getInstance().getChatServer(userInfo.uid);
                if (reply.error())
                {
                    std::cout << " grpc get char server failed, error is " << reply.error()
                              << std::endl;
                    root["error"] = ErrorCodes::RPCFAILED;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->GetResponse().body()) << jsonstr;
                    return;
                }

                std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
                root["error"] = 0;
                root["email"] = email;
                root["uid"] = userInfo.uid;
                root["token"] = reply.token();
                root["host"] = reply.host();
                root["port"] = reply.port();
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->GetResponse().body()) << jsonstr;
            });
}

bool LogicSystem::handleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
    if (_get_handlers.find(path) == _get_handlers.end())
    {
        return false;
    }
    _get_handlers[path](con);
    return true;
}

bool LogicSystem::handlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
    if (_post_handlers.find(path) == _post_handlers.end())
    {
        return false;
    }
    _post_handlers[path](con);
    return true;
}
