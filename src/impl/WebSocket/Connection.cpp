#include "../../lib/WebSocket/Connection.h"
#include "../../lib/WebSocket/ConnectionMgr.h"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/system/detail/error_code.hpp>
#include <boost/uuid/random_generator.hpp>

Connection::Connection(boost::asio::io_context& ioc)
    : _ioc(ioc)
    , _ws_ptr(std::make_unique<stream<tcp_stream>>(make_strand(ioc))) {
    // Generate random
    boost::uuids::random_generator generator;
    boost::uuids::uuid             uuid = generator();

    _uuid = boost::uuids::to_string(uuid);
}


std::string Connection::GetUUID() {
    return _uuid;
}

net::ip::tcp::socket& Connection::GetSocket() {
    auto& socket = boost::beast::get_lowest_layer(*_ws_ptr).socket();
    return socket;
}

void Connection::AsyncAccept() {
    auto self = shared_from_this();
    _ws_ptr->async_accept([self](boost::system::error_code ec) {
        try {
            if (!ec) {
                ConnectionMgr::GetInstance().AddConnection(self);
                std::cout << "AddConnection success: " << self->GetUUID()
                          << std::endl;
                self->Start();
            } else {
                std::cout << "WebSocket accept error: " << ec.message()
                          << std::endl;
            }

        } catch (std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    });
}

void Connection::Start() {
    auto self = shared_from_this();
    std::cout << "Begin async_read" << std::endl;
    _ws_ptr->async_read(
        _recv_buffer,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            try {
                if (ec) {
                    std::cout << "WebSocket read error: " << ec.message()
                              << std::endl;
                    ConnectionMgr::GetInstance().RemoveConnection(
                        self->GetUUID());
                    return;
                }
                std::cout << "Trying getting data" << std::endl;
                self->_ws_ptr->text(self->_ws_ptr->got_text());
                std::string recv_data
                    = beast::buffers_to_string(self->_recv_buffer.data());
                self->_recv_buffer.consume(self->_recv_buffer.size());
                std::cout << "WebSocket recv data: " << recv_data << std::endl;
                self->AsyncSend(std::move(recv_data));
                self->Start();
            } catch (std::exception& e) {
                std::cout << "Exception: " << e.what() << std::endl;
                ConnectionMgr::GetInstance().RemoveConnection(self->GetUUID());
            }
        });
}

void Connection::AsyncSend(std::string msg) {
    {
        std::lock_guard<std::mutex> lock(_send_mtx);
        int                         que_size = _send_que.size();
        _send_que.push(msg);
        if (que_size > 0) {
            return;
        }
    }


    auto self = shared_from_this();
    _ws_ptr->async_write(
        boost::asio::buffer(msg.c_str(), msg.length()),
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            try {
                if (!ec) {
                    std::string send_msg;
                    {
                        std::lock_guard<std::mutex> lock(self->_send_mtx);
                        self->_send_que.pop();
                        if (self->_send_que.empty()) {
                            return;
                        }
                        send_msg = self->_send_que.front();
                    }
                    self->AsyncSend(std::move(send_msg));
                } else {
                    std::cout << "WebSocket send error: " << ec.message()
                              << std::endl;
                    ConnectionMgr::GetInstance().RemoveConnection(
                        self->GetUUID());
                    return;
                }
            } catch (std::exception& e) {
                std::cout << "Exception: " << e.what() << std::endl;
            }
        });
}
