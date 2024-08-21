#pragma once
#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <queue>
#include <mutex>

namespace net = boost::asio;
namespace beast = boost::beast;
using namespace boost::beast;
using namespace boost::beast::websocket;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::io_context& ioc);
    std::string GetUUID();
    boost::asio::ip::tcp::socket& GetSocket();
    void AsyncAccept();
    void Start();
    void AsyncSend(std::string msg);
    void SendCallBack(std::string msg);
private:
    std::unique_ptr<stream<tcp_stream>> _ws_ptr;
    std::string _uuid;
    boost::asio::io_context &_ioc;
    flat_buffer _recv_buffer;
    std::queue<std::string> _send_que;
    std::mutex _send_mtx;
};

