#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include "../../out/Debug/proto/gen_cxx/msg.pb.h"
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include <memory>
#include <thread>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>

using namespace std;
using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;





// 判断当前系统的字节序是大端序还是小端序
bool is_big_endian() {
    int num = 1;
    if ( *(char *)&num == 1 ) {
        // 当前系统为小端序
        return false;
    } else {
        // 当前系统为大端序
        return true;
    }
}

void test_big_endian() {

    int num = 0x12345678;
    char *p = (char *)&num;
    cout << "原始数据：" << hex << num << endl;
    if ( is_big_endian() ) {
        cout << "当前系统为大端序" << endl;
        cout << "字节序为：";
        for ( int i = 0; i < sizeof( num ); i++ ) {
            cout << hex << (int)*( p + i ) << " ";
        }
        cout << endl;
    } else {
        cout << "当前系统为小端序" << endl;
        cout << "字节序为：";
        for ( int i = sizeof( num ) - 1; i >= 0; i-- ) {
            cout << hex << (int)*( p + i ) << " ";
        }
        cout << endl;
    }

    uint32_t host_long_value  = 0x12345678;
    uint16_t host_short_value = 0x5678;

    uint32_t network_long_value =
        boost::asio::detail::socket_ops::host_to_network_long(
            host_long_value );
    uint16_t network_short_value =
        boost::asio::detail::socket_ops::host_to_network_short(
            host_short_value );

    std::cout << "Host long value: 0x" << std::hex << host_long_value
              << std::endl;
}

void test_protobuf() {
    Book book;
    book.set_name( "CPP programming" );
    book.set_pages( 100 );
    book.set_price( 200 );
    std::string bookstr;
    book.SerializeToString( &bookstr );
    std::cout << "serialize str is " << bookstr << std::endl;
    Book book2;
    book2.ParseFromString( bookstr );
    std::cout << "book2 name is " << book2.name() << " price is "
              << book2.price() << " pages is " << book2.pages() << std::endl;
    getchar();
}

void test_json() {
    Json::Value root;
    root["id"]          = 1001;
    root["data"]        = "hello world";
    std::string request = root.toStyledString();
    std::cout << "request is" << request << std::endl;

    Json::Value root2;
    Json::Reader reader;
    reader.parse( request, root2 );
    std::cout << "message id is: " << root2["id"] << " message is "
              << root2["data"] << std::endl;
}


void test_unique_ptr() {
    std::unique_ptr<int> ptr(std::make_unique<int>(10));
    std::cout << *ptr << std::endl;
}
awaitable<void> echo(tcp::socket socket) {
    try {
        char data[1024];
        while(true) {
            size_t n = co_await socket.async_read_some(boost::asio::buffer(data), use_awaitable);
            co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
        }
    } catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

awaitable<void> listener() {
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), 10086});
    while(true) {
        tcp::socket sock = co_await acceptor.async_accept(use_awaitable );
        co_spawn(executor, echo(std::move(sock)), detached);
    }
}
int main() { 
    try {
        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto){
            io_context.stop();
        });

        co_spawn(io_context, listener(), detached);

        io_context.run();

    } catch(std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0; 
}
