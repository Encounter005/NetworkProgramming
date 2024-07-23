#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include <iostream>
using boost::asio::ip::tcp;

constexpr int RECVSIZE = 1024;

class MsgNode {
public:
    MsgNode( const char *msg, int total_len )
        : _cur_len( 0 ), _total_len( total_len ) {
        _msg = new char[_total_len];
        memcpy( _msg, msg, total_len );
    }

    MsgNode( int total_len ) : _cur_len( 0 ), _total_len( total_len ) {
        _msg = new char[_total_len];
    }

    ~MsgNode() { delete[] _msg; }

    int _cur_len;
    int _total_len;
    char *_msg;
};

class Session {
    using socket_ptr  = std::shared_ptr<tcp::socket>;
    using MsgNode_ptr = std::shared_ptr<MsgNode>;
    using Msg_Queue = std::queue<MsgNode_ptr>;

public:
    Session( socket_ptr socket );
    void Connect( const tcp::endpoint &ep );

    // async write
    void WriteCallBackErr( const boost::system::error_code &error,
        std::size_t bytes_transferred);
    void WriteToSocketErr( const std::string &buf );
    
    //async read
    void ReadFromSocket();
    void ReadCallBack(const boost::system::error_code&, size_t);



private:
    socket_ptr socket_;
    MsgNode_ptr _send_node;
    MsgNode_ptr _recv_node;
    Msg_Queue _send_queue;
    bool _send_pending;
    bool _recv_pending;
};
