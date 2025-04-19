#include "../include/httpServer.h"

namespace httpserver {

std::size_t request_count() {
    static std::size_t count = 0;
    return ++count;
}

std::time_t now() {
    return std::time(0);
}


void http_connection::read_request() {
    auto self = shared_from_this();

    http::async_read(
        socket_,
        buffer_,
        request_,
        [self](beast::error_code ec, size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (!ec) {
                self->process_request();
            }
        });
}

void http_connection::process_request() {
    response_.version(request_.version());
    response_.keep_alive(false);

    switch (request_.method()) {
    case http::verb::get:
        response_.result(http::status::ok);
        response_.set(http::field::server, "Beast");
        create_response();
        break;
    case http::verb::post:
        response_.result(http::status::ok);
        response_.set(http::field::server, "Beast");
        create_post_response();
        break;
    default:
        // We return responses indicating an error if we do not recognize the
        // request method.
        response_.result(http::status::bad_request);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body())
            << "Invalid request method: "
            << std::string(request_.method_string()) << std::endl;
        break;
    }

    write_response();
}


void http_connection::create_response() {
    if (request_.target() == "/count") {
        response_.set(http::field::content_type, "text/html");
        beast::ostream(response_.body())
            << "<html>\n"
            << "<head><title>Request count</title></head>\n"
            << "<body>\n"
            << "<h1>Request count</h1>\n"
            << "<p>There have been " << request_count()
            << " requests so far.</p>\n"
            << "</body>\n"
            << "</html>\n";
    } else if (request_.target() == "/time") {
        response_.set(http::field::content_type, "text/html");
        beast::ostream(response_.body())
            << "<html>\n"
            << "<head><title>Current time</title></head>\n"
            << "<body>\n"
            << "<h1>Current time</h1>\n"
            << "<p>The current time is " << now()
            << " seconds since the epoch.</p>\n"
            << "</body>\n"
            << "</html>\n";
    } else if (request_.target() == "/") {
        response_.set(http::field::content_type, "text/html");
        beast::ostream(response_.body())
            << "<html>\n"
            << "<head><title>The is the Main Page</title></head>\n"
            << "<body>\n"
            << "<h1>Welcome to the Main Page</h1>\n"
            << "</body>\n"
            << "</html>\n";


    } else {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body()) << "File not found\r\n";
    }
}

void http_connection::create_post_response() {
    if (request_.target() == "/email") {
        auto& body     = this->request_.body();
        auto  body_str = boost::beast::buffers_to_string(body.data());
        std::cout << "Receive body is: " << body_str << std::endl;
        this->response_.set(http::field::content_type, "text/json");
        Json::Value  root;
        Json::Reader reader;
        Json::Value  src_root;
        bool         parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
            std::cout << "Failed to parse JSON data!" << std::endl;
            root["error"]       = 1001;
            std::string jsonstr = root.toStyledString();
            beast::ostream(this->response_.body()) << jsonstr;
            return;
        }

        auto email = src_root["email"].asString();
        std::cout << "email is " << email << std::endl;

        root["error"]       = 0;
        root["email"]       = email;
        root["msg"]         = "receive email post success";
        std::string jsonstr = root.toStyledString();
        beast::ostream(this->response_.body()) << jsonstr;
    } else {
        response_.result(http::status::not_found);
        response_.set(http::field::content_type, "text/plain");
        beast::ostream(response_.body()) << "File not found\r\n";
    }
}

void http_connection::write_response() {
    auto self = shared_from_this();
    response_.content_length(response_.body().size());

    http::async_write(
        socket_,
        response_,
        [self](beast::error_code ec, size_t bytes_transferred) {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_.cancel();
        });
}

void http_connection::check_deadline() {
    auto self = shared_from_this();
    deadline_.async_wait([self](beast::error_code ec) {
        if (!ec) {
            // Close socket to cancel any outstanding operation.
            self->socket_.close(ec);
        }
    });
}

void http_server(tcp::acceptor& acceptor, tcp::socket& socket) {
    acceptor.async_accept(socket, [&](beast::error_code ec) {
        if (!ec) std::make_shared<http_connection>(std::move(socket))->start();
        http_server(acceptor, socket);
    });
}


};   // namespace httpserver
