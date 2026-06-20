#pragma once
#include "Singleton.h"
#include "ThreadPool.h"
#include <boost/asio.hpp>
#include <memory>
#include <vector>

class ThreadPoolMgr : public Singleton<ThreadPoolMgr>
{
    friend class Singleton<ThreadPoolMgr>;

public:
    ~ThreadPoolMgr();

    void enqueueHttpLogic(std::function<void()> task);
    void enqueueGrpc(std::function<void()> task);
    void enqueueMySql(std::function<void()> task);
    void enqueueRedis(std::function<void()> task);

    // HTTP IO 池：round-robin 返回一个 io_context 给新连接
    boost::asio::io_context &getIoService();

private:
    ThreadPoolMgr();

    // --- 任务线程池 ---
    std::unique_ptr<ThreadPool> _httpLogicPool;
    std::unique_ptr<ThreadPool> _grpcPool;
    std::unique_ptr<ThreadPool> _mysqlPool;
    std::unique_ptr<ThreadPool> _redisPool;

    // --- IO 线程池 ---
    using IOService = boost::asio::io_context;
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    std::size_t _ioPoolSize;
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _ioThreads;
    std::size_t _nextIoService;
};