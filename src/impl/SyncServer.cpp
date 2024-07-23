#include "../lib/SyncServer.h"
#include <memory>

SyncServer::SyncServer( boost::asio::io_context &ioc, unsigned short port )
    : port_( port ), ioc_( ioc ),
      acceptor( ioc, tcp::endpoint( tcp::v4(), port_ ) ) {}

void SyncServer::start() {
    do_accept();
    for ( auto &t : thread_set_ ) {
        if ( t->joinable() ) {
            t->join();
        }
    }
}

void SyncServer::do_accept() {
    while(true) {
        socket_ptr socket( new tcp::socket( ioc_ ) );
        acceptor.accept(*socket);
        auto func = std::bind(&SyncServer::session, this,  socket);
        auto thread = std::make_shared<std::thread>(func);
        thread_set_.insert(thread);
    }
}

void SyncServer::session( socket_ptr socket ) {
    try {
        for ( ;; ) {
            char data[MAX_LENGTH];
            memset( data, '\0', MAX_LENGTH);
            boost::system::error_code error;
            size_t length = socket->read_some(
                boost::asio::buffer( data, MAX_LENGTH ), error );
            if ( error == boost::asio::error::eof )
                break;
            else if ( error )
                throw boost::system::system_error( error );
            std::cout << data << std::endl;
            boost::asio::write( *socket, boost::asio::buffer( data, length ) );
        }
    } catch ( std::exception &e ) {
        std::cerr << e.what() << std::endl;
    }
}
