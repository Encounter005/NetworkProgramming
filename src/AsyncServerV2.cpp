#include "../include/AsyncServerV2.h"
#include <boost/asio/write_at.hpp>
#include "../build/proto/gen_cxx/transport.pb.h"


AsyncServer::AsyncServer(boost::asio::io_context &ioc, short port) : _io_context(ioc), _port(port), _acceptor(_io_context, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "Server start at port: " << port << std::endl;
    StartAccept();
}

void AsyncServer::StartAccept() {
    std::shared_ptr<Session> new_session =
        std::make_shared<Session>( _io_context, this );
    std::cout << "Create a new Session, uuid is: " << new_session->get_uuid() << std::endl;
    // NOTE:
    // 在回调的时候，new_session的引用计数+1，这样保证new_session不会被销毁
    _acceptor.async_accept(
        new_session->get_socket(), std::bind( &AsyncServer::HandleAccept, this,
                                       new_session, std::placeholders::_1 ) );
}

void AsyncServer::HandleAccept( std::shared_ptr<Session> new_session,
    const boost::system::error_code &error ) {
    if ( !error ) {
        new_session->Start();
        _sessions.insert(
            std::make_pair( new_session->get_uuid(), new_session ) );
    } else {
        std::cout << "accept failed, code is: " << error.value()
                  << " message is: " << error.message() << std::endl;
    }
    StartAccept();
}
void AsyncServer::ClearSession( const std::string &uuid ) {
    _sessions.erase( uuid );
}

Session::Session( boost::asio::io_context &ioc, AsyncServer *server )
    : _socket( ioc ), _server( server ) {
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid                     = boost::uuids::to_string( a_uuid );
}

void Session::HandleWrite( const boost::system::error_code &error,
    std::shared_ptr<Session> _self_shared ) {
    try {
        if ( !error ) {
            std::lock_guard<std::mutex> lock( _send_lock );
            _send_que.pop();
            if ( !_send_que.empty() ) {
                auto msgnode = _send_que.front();
                _socket.async_write_some(
                    boost::asio::buffer( msgnode->_msg, msgnode->_total_len ),
                    std::bind( &Session::HandleWrite, this,
                        std::placeholders::_1, _self_shared ) );
            }
        } else {
            std::cout << "handle write failed, code is: " << error.value()
                      << " message is: " << error.message() << std::endl;
            _server->ClearSession( _uuid );
        }

    } catch ( std::exception &e ) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}

void Session::HandleRead( const boost::system::error_code &error,
    size_t bytes_transferred, std::shared_ptr<Session> _self_shared ) {
    if ( !error ) {

        // std::chrono::seconds duration( 2 );
        // std::this_thread::sleep_for( duration );
        // 已经移动的字符串
        int copy_len = 0;
        while ( bytes_transferred > 0 ) {
            // 判断头部是否处理
            if ( !_b_head_parse ) {
                // 如果数据小于头部大小，先将数据放入_recv_head_node
                if ( bytes_transferred + _recv_head_node->_cur_len <
                     HEAD_LENGTH ) {
                    memcpy( _recv_head_node->_msg + _recv_head_node->_cur_len,
                        _data + copy_len, bytes_transferred );
                    _recv_head_node->_cur_len += bytes_transferred;
                    memset( _data, 0, MAX_LENGTH );
                    _socket.async_read_some(
                        boost::asio::buffer( _data, MAX_LENGTH ),
                        std::bind( &Session::HandleRead, this,
                            std::placeholders::_1, std::placeholders::_2,
                            _self_shared ) );
                    return;
                }

                // 收到的数据比头部多，可能是多个逻辑包，要做切包处理
                // 头部剩余未复制的长度
                int head_remain = HEAD_LENGTH - _recv_head_node->_cur_len;
                memcpy( _recv_head_node->_msg + _recv_head_node->_cur_len,
                    _data + copy_len, head_remain );

                // 更新已处理的data长度和剩余未处理长度
                copy_len += head_remain;
                bytes_transferred -= head_remain;
                // 获取头部数据
                short data_len = 0;
                memcpy( &data_len, _recv_head_node->_msg, HEAD_LENGTH );
                data_len =
                    boost::asio::detail::socket_ops::host_to_network_short(
                        data_len );
                std::cout << "data_len is: " << data_len << std::endl;

                // 头部非法长度
                if ( data_len > HEAD_LENGTH ) {
                    std::cout << "Invalid data length is: " << data_len
                              << std::endl;
                    _server->ClearSession( _uuid );
                    return;
                }

                _recv_msg_node = std::make_shared<MsgNode>( data_len );

                // 消息的长度小于头部规定的长度，说明数据未收集全，则先将部分消息放到接收节点里
                if ( static_cast<int>( bytes_transferred ) < data_len ) {
                    memcpy( _recv_msg_node->_msg + _recv_msg_node->_cur_len,
                        _data + copy_len, bytes_transferred );
                    _recv_msg_node->_cur_len += bytes_transferred;
                    memset( _data, 0, MAX_LENGTH );
                    _socket.async_read_some(
                        boost::asio::buffer( _data, MAX_LENGTH ),
                        std::bind( &Session::HandleRead, this,
                            std::placeholders::_1, std::placeholders::_2,
                            _self_shared ) );
                    // 头部处理完成
                    _b_head_parse = true;
                    return;
                }

                memcpy( _recv_msg_node->_msg + _recv_msg_node->_cur_len,
                    _data + copy_len, data_len );

                _recv_msg_node->_cur_len += data_len;
                copy_len += data_len;
                bytes_transferred -= data_len;
                _recv_msg_node->_msg[_recv_msg_node->_total_len] = '\0';
                // std::cout << "Recv msg is: " << _recv_msg_node->_msg
                //           << std::endl;
                // use Send for testing
                // Send( _recv_msg_node->_msg, _recv_msg_node->_total_len );
                Data msgdata;
                std::string receive_data;
                msgdata.ParseFromString( std::string(
                    _recv_msg_node->_msg, _recv_msg_node->_total_len ) );
                std::cout << "Recv msg is: " << msgdata.data() << std::endl;
                std::string return_str =
                    "server has receive msg, msg data is  " + msgdata.data();
                Data msgreturn;
                msgreturn.set_id( msgdata.id() );
                msgreturn.set_data( msgdata.data() );
                msgreturn.SerializeToString( &return_str );
                Send( return_str );
                // 继续轮询未处理的数据
                _b_head_parse = false;
                _recv_head_node->clear();
                if ( bytes_transferred <= 0 ) {
                    memset( _data, 0, MAX_LENGTH );
                    _socket.async_read_some(
                        boost::asio::buffer( _data, MAX_LENGTH ),
                        std::bind( &Session::HandleRead, this,
                            std::placeholders::_1, std::placeholders::_2,
                            _self_shared ) );
                    return;
                }
                continue;
            }

            // 已经处理完头部，处理上次未接收完的消息数据
            // 接收的数据仍不足剩余未处理的
            int remain_msg =
                _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if ( (int)bytes_transferred < remain_msg ) {
                memcpy( _recv_msg_node->_msg + _recv_msg_node->_cur_len,
                    _data + copy_len, bytes_transferred );
                _recv_msg_node->_cur_len += bytes_transferred;
                memset( _data, 0, MAX_LENGTH );
                _socket.async_read_some(
                    boost::asio::buffer( _data, MAX_LENGTH ),
                    std::bind( &Session::HandleRead, this,
                        std::placeholders::_1, std::placeholders::_2,
                        _self_shared ) );
                return;
            }

            memcpy( _recv_msg_node->_msg + _recv_msg_node->_cur_len,
                _data + copy_len, remain_msg );
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            _recv_msg_node->_msg[_recv_msg_node->_total_len] = '\0';
            // std::cout << "Recv msg is: " << _recv_msg_node->_msg <<
            // std::endl; use Send for testing Send( _recv_msg_node->_msg,
            // _recv_msg_node->_total_len );
            Data msgdata;
            std::string receive_data;
            msgdata.ParseFromString( std::string(
                _recv_msg_node->_msg, _recv_msg_node->_total_len ) );
            std::cout << "Recv msg is: " << msgdata.data() << std::endl;
            std::string return_str =
                "server has receive msg, msg data is  " + msgdata.data();
            Data msgreturn;
            msgreturn.set_id( msgdata.id() );
            msgreturn.set_data( msgdata.data() );
            msgreturn.SerializeToString( &return_str );
            Send( return_str );

            // 继续轮询未处理的数据
            _b_head_parse = false;
            _recv_head_node->clear();
            if ( bytes_transferred <= 0 ) {
                _socket.async_read_some(
                    boost::asio::buffer( _data, MAX_LENGTH ),
                    std::bind( &Session::HandleRead, this,
                        std::placeholders::_1, std::placeholders::_2,
                        _self_shared ) );
                return;
            }
            continue;
        }
    } else {
        std::cout << "handle read failed, code is: " << error.value()
                  << " message is: " << error.message() << std::endl;
        _server->ClearSession( _uuid );
    }
}

void Session::PrintRecvData( char *data, int length ) {
    std::stringstream ss;
    std::string result = "0x";
    for ( int i = 0; i < length; i++ ) {
        std::string hexstr;
        ss << std::hex << std::setw( 2 ) << std::setfill( '0' )
           << int( data[i] ) << std::endl;
        ss >> hexstr;
        result += hexstr;
    }

    std::cout << "Recv raw data is: " << result << std::endl;
}
void Session::Start() {
    memset( _data, 0, MAX_LENGTH );
    _socket.async_read_some( boost::asio::buffer( _data, MAX_LENGTH ),
        std::bind( &Session::HandleRead, this, std::placeholders::_1,
            std::placeholders::_2, shared_from_this() ) );
    Send( _data, strlen( _data ) );
}

void Session::Send( char *msg, int max_length ) {
    std::lock_guard<std::mutex> lock( _send_lock );
    int send_que_size = _send_que.size();
    if ( send_que_size > MAX_SENDQUE ) {
        std::cout << "Session: " << _uuid << " send queue fulled, size is "
                  << MAX_SENDQUE << std::endl;
        return;
    }

    _send_que.push( std::make_shared<MsgNode>( msg, max_length ) );
    if ( send_que_size > 0 ) {
        return;
    }

    auto &msgnode = _send_que.front();
    _socket.async_write_some( boost::asio::buffer( msgnode->_msg, max_length ),
        std::bind( &Session::HandleWrite, this, std::placeholders::_1,
            shared_from_this() ) );
}

void Session::Send( std::string msg ) {
    std::lock_guard<std::mutex> lock( _send_lock );
    int send_que_size = _send_que.size();
    if ( send_que_size > MAX_SENDQUE ) {
        std::cout << "Session: " << _uuid << " send queue fulled, size is "
                  << MAX_SENDQUE << std::endl;
        return;
    }

    _send_que.push( std::make_shared<MsgNode>( msg.c_str(), msg.length() ) );
    if ( send_que_size > 0 ) {
        return;
    }

    auto &msgnode = _send_que.front();
    _socket.async_send( boost::asio::buffer( msgnode->_msg,
                            static_cast<size_t>( msgnode->_total_len ) ),
        std::bind( &Session::HandleWrite, this, std::placeholders::_1,
            shared_from_this() ) );
}

tcp::socket &Session::get_socket() { return _socket; }

std::string Session::get_uuid() { return _uuid; }
