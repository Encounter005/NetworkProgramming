#include "../../lib/IOServicePoolServer/IOServicePool.h"
AsioIOServicePool::AsioIOServicePool(size_t thread_size)
    : _ioServices(thread_size), _works(thread_size), _nextIOservice(0) {

    for (size_t i = 0; i < thread_size; ++i) {
        _works[i] = std::make_unique<Work>(Work(_ioServices[i]));
    }

    for (auto &IOService : _ioServices) {
        _threads.emplace_back([this, &IOService]() { IOService.run(); });
    }
}

boost::asio::io_context &AsioIOServicePool::get_io_service() {
    auto &service = _ioServices[_nextIOservice++ % _ioServices.size()];
    return service;
}

void AsioIOServicePool::Stop() {
    for (auto &work : _works) {
        work.reset();
    }

    for (auto &t : _threads) {
        t.join();
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    std::cout << "AsioIOServicePool destroyed" << std::endl;
}
