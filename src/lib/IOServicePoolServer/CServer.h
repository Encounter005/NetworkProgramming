#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <boost/asio.hpp>
#include "CSession.h"
#include "IOServicePool.h"
using boost::asio::ip::tcp;

class CServer {
public:
    CServer(boost::asio::io_context& ioc, short port);
    void ClearSession(const std::string&);
private:
    void HandleAccept( std::shared_ptr<CSession>, const boost::system::error_code &error );
    void StartAccept();
    boost::asio::io_context &_ioc;
    short _port;
    tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<CSession>> _sessions;
};
