#include <iostream>
#include "src/SyncServer.h"

int main() {
    boost::asio::io_context ioc;
    SyncServer server(ioc, 10086);
    server.start();
    return 0;
}
