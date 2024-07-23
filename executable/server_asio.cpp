#include "../src/lib/AsyncServer.h"

int main() {
    boost::asio::io_context ioc;
    AsyncServer server(ioc, 10086);
    return 0;
}
