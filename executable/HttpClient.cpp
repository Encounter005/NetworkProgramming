#include <boost/asio.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <istream>
#include <string>

using boost::asio::ip::tcp;

class client {
public:
    client(
        boost::asio::io_context& ioc, const std::string& server,
        const std::string& path)
        : resolver_(ioc), socket_(ioc) {
        // NOTE: Form the request. We specify the "Connection: close" header so
        // that the server will close the socket after transmitting the
        // response. This will allow us to treat all data up until the EOF as
        // the content
        std::ostream request_stream(&request_);
        request_stream << "GET " << path << " HTTP/1.0\r\n";
        request_stream << "Host: " << server << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        size_t      pos  = server.find(":");
        std::string ip   = server.substr(0, pos);
        std::string port = server.substr(pos + 1);

        // Start an async resolve to translate the server and service names in
        // to a list of endpoints.
        resolver_.async_resolve(
            ip,
            port,
            boost::bind(
                &client::handle_resolve,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::results));
    }

private:
    void handle_resolve(
        const boost::system::error_code&   ec,
        const tcp::resolver::results_type& endpoints) {
        if (!ec) {
            // NOTE: attempt a connection to each endpoint in the list until
            // we successfully establish a connection.
            boost::asio::async_connect(
                socket_,
                endpoints,
                boost::bind(
                    &client::handle_connect,
                    this,
                    boost::asio::placeholders::error));
        } else {
            print_error(ec);
        }
    }

    void handle_connect(const boost::system::error_code& ec) {
        if (!ec) {
            // NOTE: The connection was successful. Send the request.
            boost::asio::async_write(
                socket_,
                request_,
                boost::bind(
                    &client::handle_write_request,
                    this,
                    boost::asio::placeholders::error));
        } else {
            print_error(ec);
        }
    }
    void handle_write_request(const boost::system::error_code& ec) {
        if (!ec) {
            // NOTE: Read the response status line. The response_ streambuf will
            // automatically grow to accommodate the entire line. The growth may
            // be limited by passing a maximum size to the streambuf
            // constructor.
            boost::asio::async_read_until(
                socket_,
                response_,
                "\r\n",
                boost::bind(
                    &client::handle_read_status_line,
                    this,
                    boost::asio::placeholders::error));
        } else {
            print_error(ec);
        }
    }
    void handle_read_status_line(const boost::system::error_code& ec) {
        if (!ec) {
            // NOTE: check the line is ok
            std::istream response_stream(&response_);
            std::string  http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                std::cout << "Invalid response" << std::endl;
                return;
            }
            if (status_code != 200) {
                std::cout << "Response returned with status code "
                          << status_code << std::endl;
                return;
            }
            // NOTE: Read the response headers, which are terminated by blank
            // line.
            boost::asio::async_read_until(
                socket_,
                response_,
                "\r\n\r\n",
                boost::bind(
                    &client::handle_read_headers,
                    this,
                    boost::asio::placeholders::error));
        } else {
            print_error(ec);
        }
    }
    void handle_read_headers(const boost::system::error_code& ec) {
        if (!ec) {
            // NOTE: Process the response headers.
            std::istream response_stream(&response_);
            std::string  header;
            while (std::getline(response_stream, header) && header != "\r") {
                std::cout << header << std::endl;
            }
            std::cout << std::endl;


            // NOTE: Write whatever content we already have to output.
            if (response_.size() > 0) {
                std::cout << &response_;
            }

            // Start reading the remaining data until EOF.
            boost::asio::async_read(
                socket_,
                response_,
                boost::asio::transfer_at_least(1),
                boost::bind(
                    &client::handle_read_content,
                    this,
                    boost::asio::placeholders::error));
        } else {
            print_error(ec);
        }
    }
    void handle_read_content(const boost::system::error_code& ec) {
        if (!ec) {
            // NOTE: Write all of the data that has been read so far.
            std::cout << &response_;
            // Continue reading remaining data until EOF.
            boost::asio::async_read(
                socket_,
                response_,
                boost::asio::transfer_at_least(1),
                boost::bind(
                    &client::handle_read_content,
                    this,
                    boost::asio::placeholders::error));
        } else if (ec != boost::asio::error::eof) {
            print_error(ec);
        }
    }
    void print_error(const boost::system::error_code& ec) {
        std::cout << "Error: " << ec.message() << " code is: " << ec.value()
                  << std::endl;
    }
    tcp::resolver          resolver_;
    tcp::socket            socket_;
    boost::asio::streambuf request_;
    boost::asio::streambuf response_;
};


int main() {
    try {
        boost::asio::io_context io_context;
        client                  client(io_context, "127.0.0.1:10086", "text/html");
        io_context.run();
        getchar();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
