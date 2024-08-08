#include "../../lib/AsyncServerV3/CServer.h"

CServer::CServer( boost::asio::io_context &io_context, short port )
    : _ioc( io_context ), _port( port ),
      _acceptor( io_context, tcp::endpoint( tcp::v4(), port ) ) {
    std::cout << "Server start success, listen on port : " << _port << std::endl;
    StartAccept();
}

void CServer::HandleAccept(
    std::shared_ptr<CSession> new_session, const boost::system::error_code &error ) {
    if ( !error ) {
        new_session->Start();
        _sessions.insert( make_pair( new_session->GetUuid(), new_session ) );
    } else {
        std::cout << "session accept failed, error is " << error.what() << std::endl;
    }

    StartAccept();
}

void CServer::StartAccept() {
    std::shared_ptr<CSession> new_session =
        std::make_shared<CSession>( _ioc, this );
    std::cout << "Create a new Session, uuid is: " << new_session->GetUuid() << std::endl;
    _acceptor.async_accept(
        new_session->GetSocket(), std::bind( &CServer::HandleAccept, this,
                                      new_session, std::placeholders::_1 ) );
}

void CServer::ClearSession(const std::string& uuid ) { _sessions.erase( uuid ); }