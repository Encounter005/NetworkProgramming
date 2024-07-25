#include "../src/lib/AsyncServerV2.h"

int main() {
    try {
        boost::asio::io_context ioc;
        AsyncServer server( ioc, 10086 );
        ioc.run();
    } catch ( std::exception &e ) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
