#include "ThreadPoolMgr.h"

ThreadPoolMgr::ThreadPoolMgr()
    : _ioPoolSize(2), _ioServices(_ioPoolSize), _works(_ioPoolSize), _nextIoService(0)
{
    _httpLogicPool = std::make_unique<ThreadPool>(3);
    _grpcPool = std::make_unique<ThreadPool>(2);
    _mysqlPool = std::make_unique<ThreadPool>(2);
    _redisPool = std::make_unique<ThreadPool>(1);

    for (std::size_t i = 0; i < _ioPoolSize; ++i)
    {
        _works[i] = std::make_unique<Work>(boost::asio::make_work_guard(_ioServices[i]));
    }
    for (std::size_t i = 0; i < _ioPoolSize; ++i)
    {
        _ioThreads.emplace_back([this, i]() {
            _ioServices[i].run();
        });
    }
}

ThreadPoolMgr::~ThreadPoolMgr()
{
    for (auto &work : _works)
    {
        work->get_executor().context().stop();
        work.reset();
    }
    for (auto &t : _ioThreads)
    {
        if (t.joinable())
            t.join();
    }
}

boost::asio::io_context &ThreadPoolMgr::getIoService()
{
    auto &service = _ioServices[_nextIoService++];
    if (_nextIoService == _ioPoolSize)
        _nextIoService = 0;
    return service;
}

void ThreadPoolMgr::enqueueHttpLogic(std::function<void()> task)
{
    _httpLogicPool->enqueue(std::move(task));
}

void ThreadPoolMgr::enqueueGrpc(std::function<void()> task)
{
    _grpcPool->enqueue(std::move(task));
}

void ThreadPoolMgr::enqueueMySql(std::function<void()> task)
{
    _mysqlPool->enqueue(std::move(task));
}

void ThreadPoolMgr::enqueueRedis(std::function<void()> task)
{
    _redisPool->enqueue(std::move(task));
}