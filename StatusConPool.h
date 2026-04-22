#pragma once
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

using grpc::Channel;

using message::StatusService;

class StatusConPool
{
public:
    StatusConPool(size_t poolSize, std::string host, std::string port);
    ~StatusConPool();
    std::unique_ptr<StatusService::Stub> getConnection();
    void returnConnection(std::unique_ptr<StatusService::Stub> context);
    void close();

private:
    std::atomic<bool> _b_stop;
    size_t _pool_size;
    std::string _host;
    std::string _port;
    std::queue<std::unique_ptr<StatusService::Stub>> _connections;
    std::mutex _mutex;
    std::condition_variable _cond;
};