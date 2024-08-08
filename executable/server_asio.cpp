#include "../src/lib/AsyncServerV3/CServer.h"

int main() {
    try {
        boost::asio::io_context ioc;
        boost::asio::signal_set signals( ioc, SIGINT, SIGTERM );
        signals.async_wait( [&ioc]( auto, auto ) { ioc.stop(); } );
        short port = 12345;
        CServer server( ioc, port );
        ioc.run();
    } catch ( std::exception &e ) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
