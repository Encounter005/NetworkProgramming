#include "../../src/lib/IOServicePoolServer/CServer.h"

int main() {
    try {
        boost::asio::io_context ioc;
        auto                    pool = AsioIOServicePool::GetInstance();
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, pool](auto, auto) {
            ioc.stop();
            pool->Stop();
        });

        CServer server(ioc, 10086);
        ioc.run();
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
