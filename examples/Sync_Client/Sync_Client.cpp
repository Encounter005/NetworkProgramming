#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <boost/asio/detail/socket_ops.hpp>
#include <iostream>
#include <chrono>
#include <thread>
using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH  = 1024 * 2;
const int HEAD_LENGTH = 2;
const int HEAD_TOTAL  = 4;

std::vector<std::thread> _threads;
int main() {
    auto start = std::chrono::high_resolution_clock::now();
    for ( int i = 0; i < 100; i++ ) {
        _threads.emplace_back( []() {
            try {
                // 创建上下文服务
                boost::asio::io_context ioc;
                // 构造endpoint
                tcp::endpoint remote_ep(
                    address::from_string( "127.0.0.1" ), 10086 );
                tcp::socket sock( ioc );
                boost::system::error_code error =
                    boost::asio::error::host_not_found;
                ;
                sock.connect( remote_ep, error );
                if ( error ) {
                    cout << "connect failed, code is " << error.value()
                         << " error msg is " << error.message();
                    return;
                }

                int i = 0;
                while ( i < 5000  ) {
                    Json::Value root;
                    root["id"]         = 1001;
                    root["data"]       = "hello world";
                    auto request       = root.toStyledString();
                    size_t request_length = request.length();

                    char send_data[MAX_LENGTH] = { 0 };
                    int msgid                  = 1001;
                    int msgid_host =
                        boost::asio::detail::socket_ops::host_to_network_short(
                            msgid );
                    memcpy( send_data, &msgid_host, 2 );
                    // 转为网络字节序
                    int request_host_length =
                        boost::asio::detail::socket_ops::host_to_network_short(
                            request_length );
                    memcpy( send_data + 2, &request_host_length, 2 );
                    memcpy( send_data + 4, request.c_str(), request_length );

                    boost::asio::write( sock,
                        boost::asio::buffer( send_data, request_length + 4 ) );
                    cout << "begin to receive..." << endl;

                    char reply_head[HEAD_TOTAL];
                    size_t reply_length = boost::asio::read( sock,
                        boost::asio::buffer( reply_head, HEAD_TOTAL ), error );

                    msgid = 0;
                    memcpy( &msgid, reply_head, HEAD_LENGTH );
                    short msglen = 0;
                    memcpy( &msglen, reply_head + 2, HEAD_LENGTH );
                    // 转为本地字节序
                    msglen =
                        boost::asio::detail::socket_ops::network_to_host_short(
                            msglen );
                    msgid =
                        boost::asio::detail::socket_ops::network_to_host_short(
                            msgid );
                    char msg[MAX_LENGTH] = { 0 };
                    size_t msg_length    = boost::asio::read(
                        sock, boost::asio::buffer( msg, msglen ) );
                    Json::Reader reader;
                    reader.parse( std::string( msg, msg_length ), root );
                    std::cout << "msg id is " << root["id"] << " msg is "
                              << root["data"] << endl;
                    i++;
                }
                
            } catch ( std::exception &e ) {
                std::cerr << "Exception: " << e.what() << std::endl;
            }
        } );
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for(auto& t : _threads){
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "elapsed time is " << elapsed.count() << " s" << std::endl;
    return 0;
}
