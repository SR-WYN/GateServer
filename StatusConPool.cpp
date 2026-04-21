#include "StatusConPool.h"
#include "message.grpc.pb.h"
#include <grpcpp/security/credentials.h>
#include <memory>

StatusConPool::StatusConPool(size_t poolSize, std::string host, std::string port)
    : _poolSize(poolSize),
      _host(host),
      _port(port),
      _b_stop(false)
{
    for (size_t i = 0; i < poolSize; i++)
    {
        std::shared_ptr<Channel> channel =
            grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
        _connections.push(StatusService::NewStub(channel));
    }
}

StatusConPool::~StatusConPool()
{
    std::lock_guard<std::mutex> lock(_mutex);
    Close();
    while (!_connections.empty())
    {
        _connections.pop();
    }
}

std::unique_ptr<StatusService::Stub> StatusConPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock,[this]{
        if (_b_stop)
        {
            return true;
        }
        return !_connections.empty();
    });
    if (_b_stop)
    {
        return nullptr;
    }
    auto context = std::move(_connections.front());
    _connections.pop();
    return context;
}

void StatusConPool::ReturnConnection(std::unique_ptr<StatusService::Stub> context)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_b_stop)
    {
        return;
    }
    _connections.push(std::move(context));
    _cond.notify_one();
}

void StatusConPool::Close()
{
    _b_stop = true;
    _cond.notify_all();
}

