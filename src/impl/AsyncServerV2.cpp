#include "../lib/AsyncServerV2.h"

void AsyncServer::StartAccept() {
    std::shared_ptr<Session> new_session =
        std::make_shared<Session>( _io_context, this );
    // NOTE:
    // 在回调的时候，new_session的引用计数+1，这样保证new_session不会被销毁
    _acceptor.async_accept(
        new_session->get_socket(), std::bind( &AsyncServer::HandleAccept, this,
                                       new_session, std::placeholders::_1 ) );
}

void AsyncServer::HandleAccept( std::shared_ptr<Session> new_session,
    const boost::system::error_code &error ) {
    if ( !error ) {
        new_session->Start();
        _sessions.insert(
            std::make_pair( new_session->get_uuid(), new_session ) );
    } else {
        std::cout << "accept failed, code is: " << error.value()
                  << " message is: " << error.message() << std::endl;
    }
    StartAccept();
}
void AsyncServer::ClearSession( const std::string &uuid ) {
    _sessions.erase( uuid );
}

Session::Session( boost::asio::io_context &ioc, AsyncServer *server )
    : _socket( ioc ), _server( server ) {
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid                     = boost::uuids::to_string( a_uuid );
}

void Session::HandleWrite( const boost::system::error_code &error,
    std::shared_ptr<Session> _self_shared ) {
    if ( !error ) {
        std::lock_guard<std::mutex> lock( _send_lock );
        _send_que.pop();
        if ( !_send_que.empty() ) {
            auto msgnode = _send_que.front();
            boost::asio::async_write( _socket,
                boost::asio::buffer( msgnode->_msg, msgnode->_total_len ),
                std::bind( &Session::HandleWrite, this, std::placeholders::_1,
                    _self_shared ) );
        }
    } else {
        std::cout << "handle write failed, code is: " << error.value()
                  << " message is: " << error.message() << std::endl;
        _server->ClearSession( _uuid );
    }
}

void Session::HandleRead( const boost::system::error_code &error,
    size_t bytes_transferred, std::shared_ptr<Session> _self_shared ) {
    if ( !error ) {
        std::cout << "Read data is: " << _data << std::endl;
        Send( _data, bytes_transferred );
        memset( _data, 0, MAX_LENGTH );
    } else {
        std::cout << "handle read failed, code is: " << error.value()
                  << " message is: " << error.message() << std::endl;
        _server->ClearSession( _uuid );
    }
}

void Session::Start() {
    memset( _data, 0, MAX_LENGTH );
    _socket.async_read_some( boost::asio::buffer( _data, MAX_LENGTH ),
        std::bind( &Session::HandleRead, this, std::placeholders::_1,
            std::placeholders::_2, shared_from_this() ) );
    Send( _data, strlen( _data ) );
}

void Session::Send( char *msg, int max_length ) {
    bool pending = false;
    std::lock_guard<std::mutex> lock( _send_lock );
    if ( !_send_que.empty() ) {
        pending = true;
    }
    _send_que.push( std::make_shared<MsgNode>( msg, max_length ) );
    if ( pending ) {
        return;
    }

    boost::asio::async_write( _socket, boost::asio::buffer( msg, max_length ),
        std::bind( &Session::HandleWrite, this, std::placeholders::_1,
            shared_from_this() ) );
}

tcp::socket &Session::get_socket() { return _socket; }

std::string Session::get_uuid() { return _uuid; }
