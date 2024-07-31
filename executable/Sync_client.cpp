#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
using namespace std;
using boost::asio::ip::tcp;
constexpr int MAX_LENGTH  = 1024 * 2;
constexpr int HEAD_LENGTH = 2;

int main() {
    try {
        boost::asio::io_context ioc;
        tcp::endpoint remote_ep(
            boost::asio::ip::address::from_string( "127.0.0.1" ), 10086 );
        tcp::socket sock( ioc );
        boost::system::error_code ec = boost::asio::error::host_not_found;
        sock.connect( remote_ep, ec );
        if ( ec ) {
            cout << "connect failed: " << ec.message() << endl;
            return 0;
        }

        std::vector<char *> msgs = { "hello world!", "genshin impact", "Kamen Rider", "made the force be with you", "awdawdawdawdawdawdawd" };
        unsigned long index = 0;
        std::thread send_thread( [&]() {
            while ( true ) {
                // this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                const char *request_msg = msgs[(index++) % msgs.size()];

                size_t request_len         = strlen( request_msg );
                // request_len = boost::asio::detail::socket_ops::host_to_network_short(request_len);
                char send_data[MAX_LENGTH] = { 0 };
                memcpy( send_data, &request_len, 2 );
                memcpy( send_data + 2, request_msg, request_len );
                std::cout << "Send msg is: ";
                std::cout.write(request_msg, request_len);
                std::cout << std::endl;
                boost::asio::write(
                    sock, boost::asio::buffer( send_data, request_len + 2 ) );
            }
        } );

        std::thread recv_thread( [&]() {
            while ( true ) {
                // this_thread::sleep_for(std::chrono::milliseconds(1));
                std::cout << "Begin to receive" << std::endl;
                char reply_head[HEAD_LENGTH];
                size_t reply_length = boost::asio::read(
                    sock, boost::asio::buffer( reply_head, HEAD_LENGTH ) );
                short msglen = 0;
                memcpy( &msglen, reply_head, HEAD_LENGTH );
                char msg[MAX_LENGTH] = { 0 };
                size_t msg_length    = boost::asio::read(
                    sock, boost::asio::buffer( msg, msglen ) );
                cout << "Reply is: ";
                cout.write( msg, msg_length ) << endl;
                cout << "Reply length is: " << msg_length << endl;
            }
        } );

        send_thread.join();
        recv_thread.join();

    } catch ( std::exception &e ) {
        cerr << e.what() << endl;
    }

    return 0;
}
