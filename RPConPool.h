#pragma once

#include "const.h"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include <atomic>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::VerifyService;

class RPConPool
{
public:
    RPConPool(size_t poolsize, std::string host, std::string port);
    ~RPConPool();
    void close();

    std::unique_ptr<VerifyService::Stub> getConnection();
    void returnConnection(std::unique_ptr<VerifyService::Stub> context);

private:
    std::atomic<bool> _b_stop;
    size_t _pool_size;
    std::string _host;
    std::string _port;
    std::queue<std::unique_ptr<VerifyService::Stub>> _connections;
    std::condition_variable _cond;
    std::mutex _mutex;
};