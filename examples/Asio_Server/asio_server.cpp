#include "../../include/AsyncServerV2.h"
std::mutex              quit;
bool                    bstop = false;
std::condition_variable cond_quit;
int                     main() {
    try {
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        short   port = 12345;
        AsyncServer server(ioc, port);
        ioc.run();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}


