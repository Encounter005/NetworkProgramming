#pragma once
#include <boost/unordered_map.hpp>
#include "Connection.h"

class ConnectionMgr {
public:
    ~ConnectionMgr() = default;
    static ConnectionMgr& GetInstance();
    void AddConnection(std::shared_ptr<Connection> conptr);
    void RemoveConnection(std::string uuid);
private:
    ConnectionMgr(const ConnectionMgr&) = delete;
    ConnectionMgr& operator=(const ConnectionMgr&) = delete;
    ConnectionMgr() = default;
    boost::unordered_map<std::string, std::shared_ptr<Connection>> _map_cons;
};
