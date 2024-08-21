#include "../../lib/WebSocket/WebSocketServer.h"
#include <boost/asio/io_context.hpp>

void WebSocketServer::StartAccept() {
    auto con_ptr = std::make_shared<Connection>(_ioc);
    _acceptor.async_accept(con_ptr->GetSocket(), [this, con_ptr](beast::error_code ec){
        try{
            if(!ec) {
                con_ptr->AsyncAccept();
            } else {
                std::cout << "WebSocket accept error: " << ec.message() << std::endl;
            }

            StartAccept();
        } catch(std::exception &e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }
    });
}

WebSocketServer::WebSocketServer(net::io_context &ioc, unsigned short port) : _ioc(ioc), _acceptor(ioc, net::ip::tcp::endpoint(net::ip::tcp::v4(), port)) {
    std::cout << "WebSocketServer start at: " << port << std::endl;
}
