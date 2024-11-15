#include "../../src/lib/WebSocket/WebSocketServer.h"

int main() {
    try{
        net::io_context ioc;
        WebSocketServer server(ioc, 10086);
        server.StartAccept();
        ioc.run();
    } catch(std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
