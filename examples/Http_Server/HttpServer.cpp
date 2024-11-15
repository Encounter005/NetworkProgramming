#include "../../src/lib/httpServer.h"
using namespace httpserver;
int main() {
    try {
        unsigned short  port    = 10086;
        auto const      address = net::ip::make_address("0.0.0.0");
        net::io_context io_context{1};
        tcp::acceptor   acceptor(io_context, {address, port});
        tcp::socket     socket(io_context);
        http_server(acceptor, socket);
        io_context.run();
    } catch (std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
