#pragma once
#include <boost/asio/io_context.hpp>
#include <thread>
#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include "Singleton.hpp"


// NOTE:
//该线程池只需要在主函数创建服务器的时候，传入这个线程池的io_context即可
//并且主函数需要加锁

class AsioIOThreadPool : public Singleton<AsioIOThreadPool> {
    friend class Singleton<AsioIOThreadPool>;
public:
    ~AsioIOThreadPool();
    AsioIOThreadPool(const AsioIOThreadPool&) = delete;
    AsioIOThreadPool& operator=(const AsioIOThreadPool&) = delete;

    boost::asio::io_context& get_io_service();
    void Stop();
private:
    explicit AsioIOThreadPool(size_t thread_size = std::thread::hardware_concurrency());
    boost::asio::io_context _service;
    std::unique_ptr<boost::asio::io_context::work> _work;
    std::vector<std::thread> _threads;
};


