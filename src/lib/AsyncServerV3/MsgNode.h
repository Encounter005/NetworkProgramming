#pragma once
#include <iostream>
#include <cstring>
class MsgNode {
public:
    explicit MsgNode(short length) : _total_len(length) {
        _data = new char[_total_len + 1]();
        _data[_total_len] = '\0';
        _cur_len = 0;
    }

    void Clear() {
        memset(_data, 0 , _total_len);
        _cur_len = 0;
    }
    
    ~MsgNode() {
        std::cout << "destruct Msgnode" << std::endl;
        delete[] _data;
    }

    short _cur_len;
    short _total_len;
    char *_data;
};



class RecvNode : public MsgNode {
public:
    explicit RecvNode(short max_len, short msg_id);
    short _msg_id;
};

class SendNode : public MsgNode {
public:
    explicit SendNode(const char *msg, short msg_length, short msg_id);
private:
    short _msg_id;
};

