#include "RPConPool.h"
#include <memory>

RPConPool::RPConPool(size_t poolsize, std::string host, std::string port)
    : _poolSize(poolsize),
      _host(host),
      _port(port),
      _b_stop(false)
{
    for (size_t i = 0; i < _poolSize; i++)
    {
        std::shared_ptr<Channel> channel =
            grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
        _connections.push(VerifyService::NewStub(channel));
    }
}

RPConPool::~RPConPool()
{
    Close();
    std::lock_guard<std::mutex> lock(_mutex);
    while (!_connections.empty())
    {
        _connections.pop();
    }
}

void RPConPool::Close()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _b_stop = true;
    _cond.notify_all();
}

std::unique_ptr<VerifyService::Stub> RPConPool::GetConnection()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cond.wait(lock,
              [this]()
              {
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

void RPConPool::ReturnConnection(std::unique_ptr<VerifyService::Stub> context)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_b_stop)
    {
        return;
    }

    _connections.push(std::move(context));
    _cond.notify_one();
}
