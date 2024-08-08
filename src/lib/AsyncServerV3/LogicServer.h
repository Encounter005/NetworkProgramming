#pragma once
#include <json/value.h>
#include <json/reader.h>
#include <json/json.h>
#include <mutex>
#include <thread>
#include <map>
#include <queue>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include "Singleton.hpp"
#include "CSession.h"


typedef std::function<void(std::shared_ptr<CSession>, short msg_id, std::string msg_data)> FunCallBack;

class LogicSystem : public Singleton<LogicSystem> {
    friend class ::Singleton<LogicSystem>;
public:
    ~LogicSystem();
    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
    LogicSystem();
    void DealMsg();
    void ProcessOneMessage();
    void RegisterCallBacks();
    void HelloWordCallBack(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg_data);
    std::thread _worker_thread;
    std::mutex _mutex;
    std::queue<std::shared_ptr<LogicNode>> _msg_que;
    std::condition_variable _consume; //表示消费者条件变量，用来控制当逻辑队列为空时保证线程暂时挂起等待，不要干扰其他线程。
    bool _b_stop;
    std::map<short, FunCallBack> _func_callbacks; //表示回调函数的map，根据id查找对应的逻辑处理函数。
};
