#pragma once

#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>
#include <map>
#include <queue>
#include <mutex>
#include <iostream>
using boost::asio::ip::tcp;
#define MAX_LENGTH 1024
#define HEAD_LENGTH 2

class AsyncServer;

class MsgNode {
public:
    friend class Session;
    MsgNode( char *msg, short max_len )
        : _cur_len( 0 ), _total_len( max_len + HEAD_LENGTH ) {
        _msg = new char[_total_len + 1]();     // 这里➕1是为了存放'\0'
        memcpy( _msg, &max_len, HEAD_LENGTH ); // 留出两个字节存储消息头
        mempcpy( _msg + HEAD_LENGTH, msg, max_len ); // 存储消息体
        _msg[_total_len] = '\0';
    }
    MsgNode( short max_len ) : _cur_len( 0 ), _total_len( max_len ) {
        _msg = new char[_total_len + 1]();
    }

    void clear() {
        memset( _msg, 0, _total_len );
        _cur_len = 0;
    }

    ~MsgNode() { delete[] _msg; }

private:
    short _cur_len;   // 当前已处理的数据长度
    short _total_len; // 数据的总长度
    char *_msg;       // 存储的数据指针
};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session( boost::asio::io_context &ioc, AsyncServer *server );
    ~Session() { std::cout << "Session destroyed" << std::endl; }
    tcp::socket &get_socket();
    std::string get_uuid();
    void Start();
    void Send( char *msg, int max_length );

private:
    void HandleRead( const boost::system::error_code &error,
        size_t bytes_transferred, std::shared_ptr<Session> _self_shard );
    void HandleWrite( const boost::system::error_code &error,
        std::shared_ptr<Session> _self_shard );
    tcp::socket _socket;
    std::string _uuid;
    char _data[MAX_LENGTH]; // 已接收或已发送的数据
    AsyncServer *_server;
    std::queue<std::shared_ptr<MsgNode>> _send_que;
    std::mutex _send_lock;
};

class AsyncServer {
public:
    AsyncServer( boost::asio::io_context &ioc, short port );
    void ClearSession( const std::string & );

private:
    void HandleAccept(
        std::shared_ptr<Session>, const boost::system::error_code &error );
    void StartAccept();
    boost::asio::io_context &_io_context;
    short _port;
    tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<Session>> _sessions;
};
