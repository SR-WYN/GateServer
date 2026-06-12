// StatusConPool.h - StatusServer 的 gRPC 连接池，管理 StatusService Stub 的复用
#pragma once
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <atomic>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

using grpc::Channel;

using message::StatusService;

// StatusServer 连接池：预创建多个 Stub，避免每次请求都新建连接
class StatusConPool
{
public:
    StatusConPool(size_t poolSize, std::string host, std::string port);
    ~StatusConPool();
    // 从池中获取一个 Stub，池空时阻塞等待
    std::unique_ptr<StatusService::Stub> getConnection();
    // 归还 Stub 到池中
    void returnConnection(std::unique_ptr<StatusService::Stub> context);
    void close();

private:
    std::atomic<bool> _stop;
    size_t _pool_size;
    std::string _host;
    std::string _port;
    std::queue<std::unique_ptr<StatusService::Stub>> _connections;
    std::mutex _mutex;
    std::condition_variable _cond;
};
