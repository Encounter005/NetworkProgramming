#pragma once
// NOTE:
// 如果要使用这个线程池的话，
// 将CServer的io_context替换为AsioIOServicePool::GetInstance()->get_io_service()
// main函数需要给服务器创建一个io_context

#include <thread>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include "Singleton.hpp"
class AsioIOServicePool : public Singleton<AsioIOServicePool> {
    friend class Singleton<AsioIOServicePool>;

public:
    using IOService = boost::asio::io_context;
    using Work      = boost::asio::io_context::work;
    using WorkPtr   = std::unique_ptr<Work>;
    ~AsioIOServicePool();
    AsioIOServicePool( const AsioIOServicePool & )            = delete;
    AsioIOServicePool &operator=( const AsioIOServicePool & ) = delete;

    boost::asio::io_context &get_io_service();
    void Stop();

private:
    explicit AsioIOServicePool(
        size_t thread_size = std::thread::hardware_concurrency() );
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOservice; // 轮询索引
};
