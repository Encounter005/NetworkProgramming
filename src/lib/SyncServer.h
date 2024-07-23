#pragma once

#include <memory>
#include <iostream>
#include <set>
#include <thread>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class SyncServer {
public:
    using socket_ptr = std::shared_ptr<tcp::socket>;
    explicit SyncServer(boost::asio::io_context& io_context, unsigned short );
    void start();
private:
    void do_accept();
    void session(socket_ptr);
    unsigned short port_;
    std::set<std::shared_ptr<std::thread>> thread_set_;
    boost::asio::io_context &ioc_;
    tcp::acceptor acceptor;
    static constexpr size_t MAX_LENGTH = 1024;
};
