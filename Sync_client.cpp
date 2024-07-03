#include <boost/asio.hpp>
#include <boost/asio/error.hpp>
#include <iostream>
using namespace std;
using boost::asio::ip::tcp;
const int MAX_LENGTH = 1024;

int main() {
    try {
        boost::asio::io_context ioc;
        tcp::endpoint remote_ecp(
            boost::asio::ip::address::from_string( "127.0.0.1" ), 10086 );
        tcp::socket sock( ioc );
        boost::system::error_code error_code =
            boost::asio::error::host_not_found;
        sock.connect( remote_ecp, error_code );
        if ( error_code ) {
            cout << "connect failed, code is: " << error_code.value()
                 << " error message is: " << error_code.message() << endl;
            return 0;
        }

        cout << "Enter a message: ";
        char request[MAX_LENGTH];
        cin.getline( request, MAX_LENGTH );
        size_t request_length = strlen( request );
        boost::asio::write(
            sock, boost::asio::buffer( request, request_length ) );

        char reply[MAX_LENGTH];
        size_t reply_length =
            boost::asio::read( sock, boost::asio::buffer( reply, request_length ) );
        cout << "Reply is: ";
        cout.write( reply, reply_length );
        cout << endl;

    } catch ( std::exception &e ) {
        cerr << e.what() << endl;
    }

    return 0;
}
