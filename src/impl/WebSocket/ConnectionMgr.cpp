#include "../../lib/WebSocket/ConnectionMgr.h"


ConnectionMgr& ConnectionMgr::GetInstance() {
    static ConnectionMgr instance;
    return instance;
}

void ConnectionMgr::AddConnection(std::shared_ptr<Connection> conptr) {
    _map_cons[conptr->GetUUID()] = conptr;
}

void ConnectionMgr::RemoveConnection(std::string  uuid) {
    _map_cons.erase(uuid);
}
