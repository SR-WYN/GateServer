// RPConPool.h - VerifyServer 的 gRPC 连接池，管理 VerifyService Stub 的复用
#pragma once

#include "message.grpc.pb.h"
#include "message.pb.h"
#include <atomic>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <mutex>
#include <queue>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::VerifyService;

// VerifyServer 连接池：预创建多个 Stub，避免每次请求都新建连接
class RPConPool
{
public:
    RPConPool(size_t poolsize, std::string host, std::string port);
    ~RPConPool();
    void close();

    // 从池中获取一个 Stub，池空时阻塞等待
    std::unique_ptr<VerifyService::Stub> getConnection();
    // 归还 Stub 到池中，供其他请求复用
    void returnConnection(std::unique_ptr<VerifyService::Stub> context);

private:
    std::atomic<bool> _stop;
    size_t _pool_size;
    std::string _host;
    std::string _port;
    std::queue<std::unique_ptr<VerifyService::Stub>> _connections;
    std::condition_variable _cond;
    std::mutex _mutex;
};
