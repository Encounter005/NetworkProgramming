#include "../lib/AsyncServerV0.h"
#include <locale>

void Session::Connect(const tcp::endpoint &ep) {
    this->socket_->connect(ep);
}
void Session::WriteToSocketErr( const std::string &buf ) {
    _send_queue.emplace(
        std::make_shared<MsgNode>( buf.c_str(), buf.length() ) );
    // NOTE: pending状态表示上一次有未发送完的数据
    if ( _send_pending ) {
        return;
    }

    this->socket_->async_send( boost::asio::buffer( buf ),
        std::bind( &Session::WriteCallBackErr, this, std::placeholders::_1,
            std::placeholders::_2 ) );
    _send_pending = true;
}

void Session::WriteCallBackErr(
    const boost::system::error_code &error, std::size_t bytes_transferred ) {

    if ( error.value() != 0 ) {
        std::cerr << "Error, code is: " << error.value()
                  << " Message is: " << error.message() << std::endl;
        return;
    }

    auto &send_data = _send_queue.front();
    send_data->_cur_len += bytes_transferred;
    // 检查数据是否已经发送完，如果没有发送完就进行回调
    if ( send_data->_cur_len < send_data->_total_len ) {

        // NOTE: 关于这里的buffer:
        // 第一个参数是已发送的数据长度 _msg + _cur_len(偏移量)
        // 第二个参数是未发送的数据长度 _total_len - _cur_len （剩余长度）
        this->socket_->async_send(
            boost::asio::buffer( _send_node->_msg + _send_node->_cur_len,
                _send_node->_total_len - _send_node->_cur_len ),
            std::bind( &Session::WriteCallBackErr, this, std::placeholders::_1,
                std::placeholders::_2 ) );

        return;
    }

    // 如果发送完就弹出队列
    _send_queue.pop();

    if ( _send_queue.empty() ) {
        _send_pending = false;
    }

    if ( !_send_queue.empty() ) {
        auto &need_to_send = _send_queue.front();
        this->socket_->async_send(
            boost::asio::buffer( need_to_send->_msg + need_to_send->_cur_len,
                need_to_send->_total_len - need_to_send->_cur_len ),
            std::bind( &Session::WriteCallBackErr, this, std::placeholders::_1,
                std::placeholders::_2 ) );
    }
}

void Session::ReadFromSocket() {
    if ( _recv_pending ) {
        return;
    }

    //可以调用构造函数直接构造，但不可用已经构造好的智能指针赋值
    /*auto _recv_nodez = std::make_unique<MsgNode>(RECVSIZE);
    _recv_node = _recv_nodez;*/

    _recv_node = std::make_shared<MsgNode>( RECVSIZE );
    this->socket_->async_receive(
        boost::asio::buffer( _recv_node->_msg, _recv_node->_total_len ),
        std::bind( &Session::ReadCallBack, this, std::placeholders::_1,
            std::placeholders::_2 ) );

        _recv_pending = true;
}

void Session::ReadCallBack(const boost::system::error_code &error, size_t bytes_transferred) {
    _recv_node->_cur_len += bytes_transferred;
    //将数据投递到队列里交给逻辑线程处理，此处略去
    //如果读完了则将标记置为false
    _recv_pending = false;
    _recv_node = nullptr;
}
