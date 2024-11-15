#include "../../src/lib/IOThreadPoolServer/CServer.h"

int main() {
    try {
        auto                    pool = AsioIOThreadPool::GetInstance();
        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([pool, &io_context](auto, auto) {
            io_context.stop();
            pool->Stop();
        });

        CServer s(pool->get_io_service(), 10086);

        // io_context
        io_context.run();
        std::cout << "server exited " << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
}
