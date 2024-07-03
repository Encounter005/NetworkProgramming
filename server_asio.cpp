#include <boost/asio.hpp>
#include <iostream>
#include "src/Session.h"

int main() {
    try {
        boost::asio::io_context ioc;
        using namespace std;
        Server server( ioc, 8888 );
        ioc.run();
    } catch ( std::exception &e ) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
