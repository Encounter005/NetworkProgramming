#pragma once
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <memory>
#include <string>
#include <utility>


namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = boost::asio::ip::tcp;


namespace httpserver {

std::size_t request_count();
std::time_t now();

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    explicit http_connection(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() {
        read_request();
        check_deadline();
    }

private:
    tcp::socket socket_;   // The socket for the currently connected client.
    beast::flat_buffer buffer_{8192};   // The buffer for performing reads.
    http::request<http::dynamic_body>  request_;    // The request message.
    http::response<http::dynamic_body> response_;   // The response message.
    net::steady_timer                  deadline_{
        socket_.get_executor(),
        std::chrono::seconds(60)};   // The timer for putting a deadline on
                                                      // connection processing.

    void read_request();   // Asynchronously receive a complete request message.
    void process_request();   // Determine what needs to be done with the
                              // request message.
    void create_response();   // Construct a response messaeg based on the
                              // program state.
    void create_post_response();
    void write_response();   // Asynchronously transmit the response message.
    void check_deadline();   // Check whether we have spent enough time on this
                             // connection
};
void http_server(tcp::acceptor& acceptor, tcp::socket& socket);

};   // namespace httpserver
