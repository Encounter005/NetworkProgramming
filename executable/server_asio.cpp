// #include "../src/lib/IOThreadPoolServer/CServer.h"
// #include "lib/IOThreadPoolServer/IOThreadPool.h"
#include "../src/lib/CoroutineServer/CServer.h"
#include <condition_variable>
std::mutex quit;
bool bstop = false;
std::condition_variable cond_quit;
int main() {
    try {
        auto pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals( ioc, SIGINT, SIGTERM );
        signals.async_wait( [&ioc, pool]( auto, auto ) {
            ioc.stop();
            pool->Stop();
            std::unique_lock<std::mutex> lock( quit );
            bstop = true;
            cond_quit.notify_one();
        } );
        short port = 12345;
        CServer server( ioc, port );
        // {
        //     std::unique_lock<std::mutex> lock( quit );
        //     cond_quit.wait( lock, [&]() { return bstop; } );
        // }
        ioc.run();
    } catch ( std::exception &e ) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
