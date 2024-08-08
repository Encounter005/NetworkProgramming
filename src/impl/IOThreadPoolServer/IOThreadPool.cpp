#include "../../lib/IOThreadPoolServer/IOThreadPool.h"
#include <boost/asio/io_context.hpp>

AsioIOThreadPool::AsioIOThreadPool( size_t thread_size )
    : _work( std::make_unique<boost::asio::io_context::work>( _service ) ) {
    for ( size_t i = 0; i < thread_size; ++i ) {
        _threads.emplace_back( [this]() { _service.run(); } );
    }
}

boost::asio::io_context &AsioIOThreadPool::get_io_service() { return _service; }

void AsioIOThreadPool::Stop() {
    _work.reset();
    for ( auto &thread : _threads ) {
        thread.join();
    }
}

AsioIOThreadPool::~AsioIOThreadPool() {
    std::cout << "IOThreadPool destroy" << std::endl;
}
