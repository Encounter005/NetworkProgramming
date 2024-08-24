#include "../../lib/IOServicePoolServer/CSession.h"
#include "../../lib/IOServicePoolServer/CServer.h"
#include "../../lib/IOServicePoolServer/LogicServer.h"

CSession::CSession(boost::asio::io_context &io_context, CServer *server)
    : _socket(io_context)
    , _server(server)
    , _b_close(false)
    , _b_head_parse(false) {
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid                     = boost::uuids::to_string(a_uuid);
    _recv_head_node           = make_shared<MsgNode>(HEAD_TOTAL_LEN);
}
CSession::~CSession() {
    std::cout << "~CSession destruct" << endl;
}

tcp::socket &CSession::GetSocket() {
    return _socket;
}

std::string &CSession::GetUuid() {
    return _uuid;
}

void CSession::Start() {
    memset(_data, 0, MAX_LENGTH);
    _socket.async_read_some(
        boost::asio::buffer(_data, MAX_LENGTH),
        std::bind(
            &CSession::HandleRead,
            this,
            std::placeholders::_1,
            std::placeholders::_2,
            SharedSelf()));
}

void CSession::Send(std::string msg, short msgid) {
    std::lock_guard<std::mutex> lock(_send_lock);
    int                         send_que_size = _send_que.size();
    if (send_que_size > MAX_SENDQUE) {
        std::cout << "session: " << _uuid << " send que fulled, size is "
                  << MAX_SENDQUE << endl;
        return;
    }

    _send_que.push(make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
    if (send_que_size > 0) {
        return;
    }
    auto &msgnode = _send_que.front();
    boost::asio::async_write(
        _socket,
        boost::asio::buffer(msgnode->_data, msgnode->_total_len),
        std::bind(
            &CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(char *msg, short max_length, short msgid) {
    std::lock_guard<std::mutex> lock(_send_lock);
    int                         send_que_size = _send_que.size();
    if (send_que_size > MAX_SENDQUE) {
        std::cout << "session: " << _uuid << " send que fulled, size is "
                  << MAX_SENDQUE << endl;
        return;
    }

    _send_que.push(make_shared<SendNode>(msg, max_length, msgid));
    if (send_que_size > 0) {
        return;
    }
    auto &msgnode = _send_que.front();
    boost::asio::async_write(
        _socket,
        boost::asio::buffer(msgnode->_data, msgnode->_total_len),
        std::bind(
            &CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close() {
    _socket.close();
    _b_close = true;
}

std::shared_ptr<CSession> CSession::SharedSelf() {
    return shared_from_this();
}

void CSession::HandleWrite(
    const boost::system::error_code &error,
    std::shared_ptr<CSession>        shared_self) {
    // 增加异常处理
    try {
        if (!error) {
            std::lock_guard<std::mutex> lock(_send_lock);
            // cout << "send data " << _send_que.front()->_data+HEAD_LENGTH <<
            // endl;
            _send_que.pop();
            if (!_send_que.empty()) {
                auto &msgnode = _send_que.front();
                boost::asio::async_write(
                    _socket,
                    boost::asio::buffer(msgnode->_data, msgnode->_total_len),
                    std::bind(
                        &CSession::HandleWrite,
                        this,
                        std::placeholders::_1,
                        shared_self));
            }
        } else {
            std::cout << "handle write failed, error is " << error.what()
                      << endl;
            Close();
            _server->ClearSession(_uuid);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception code : " << e.what() << endl;
    }
}

void CSession::HandleRead(
    const boost::system::error_code &error, size_t bytes_transferred,
    std::shared_ptr<CSession> shared_self) {
    try {
        if (!error) {
            int copy_len = 0;
            while (bytes_transferred > 0) {
                if (!_b_head_parse) {
                    if (!ParseHeader(bytes_transferred, copy_len)) {
                        return;
                    }
                } else {
                    if (!ParseMessage(bytes_transferred, copy_len)) {
                        return;
                    }
                }
            }
            // 继续读取更多数据
            ::memset(_data, 0, MAX_LENGTH);
            _socket.async_read_some(
                boost::asio::buffer(_data, MAX_LENGTH),
                std::bind(
                    &CSession::HandleRead,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2,
                    shared_self));
        } else {
            std::cout << "handle read failed, error is " << error.what()
                      << endl;
            Close();
            _server->ClearSession(_uuid);
        }
    } catch (std::exception &e) {
        std::cout << "Exception code is " << e.what() << endl;
    }
}

bool CSession::ParseHeader(size_t &bytes_transferred, int &copy_len) {
    // std::cout << "bytes_transferred is: " << bytes_transferred << " copy_len
    // is: " << copy_len << std::endl;
    if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN) {
        memcpy(
            _recv_head_node->_data + _recv_head_node->_cur_len,
            _data + copy_len,
            bytes_transferred);
        _recv_head_node->_cur_len += bytes_transferred;
        return false;
    }

    int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
    memcpy(
        _recv_head_node->_data + _recv_head_node->_cur_len,
        _data + copy_len,
        head_remain);
    copy_len += head_remain;
    bytes_transferred -= head_remain;

    short msg_id = 0;
    memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
    msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
    std::cout << "msg_id is " << msg_id << endl;

    if (msg_id > MAX_LENGTH) {
        std::cout << "invalid msg_id is " << msg_id << endl;
        _server->ClearSession(_uuid);
        return false;
    }

    short msg_len = 0;
    memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
    msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
    std::cout << "msg_len is " << msg_len << endl;

    if (msg_len > MAX_LENGTH) {
        std::cout << "invalid data length is " << msg_len << endl;
        _server->ClearSession(_uuid);
        return false;
    }

    _recv_msg_node = make_shared<RecvNode>(msg_len, msg_id);
    _b_head_parse  = true;
    return true;
}

bool CSession::ParseMessage(size_t &bytes_transferred, int &copy_len) {
    int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
    // std::cout << "bytes_transferred is: " << bytes_transferred << " copy_len
    // is: " << copy_len << std::endl;
    if (bytes_transferred < remain_msg) {
        memcpy(
            _recv_msg_node->_data + _recv_msg_node->_cur_len,
            _data + copy_len,
            bytes_transferred);
        _recv_msg_node->_cur_len += bytes_transferred;
        return false;
    }

    memcpy(
        _recv_msg_node->_data + _recv_msg_node->_cur_len,
        _data + copy_len,
        remain_msg);
    _recv_msg_node->_cur_len += remain_msg;
    bytes_transferred -= remain_msg;
    copy_len += remain_msg;
    _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
    std::cout << "receive data is " << _recv_msg_node->_data << endl;
    LogicSystem::GetInstance()->PostMsgToQue(
        make_shared<LogicNode>(shared_from_this(), _recv_msg_node));

    _b_head_parse = false;
    _recv_head_node->Clear();
    return true;
}
LogicNode::LogicNode(
    shared_ptr<CSession> session, shared_ptr<RecvNode> recvnode)
    : _session(session), _recvnode(recvnode) {}
