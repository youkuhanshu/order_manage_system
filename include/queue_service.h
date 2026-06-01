#pragma once
#include <iostream>
#include <string>
#include <vector>
#include"queue_msg.hpp"
#include <functional>
#include <fstream>

using QueueCallback = std::function<void()>;
#define time_threshold 6000 //单位是秒

class QueueService{
private:
    int current_eating;
    int next_giving;
    std::vector<QueueMsg> waiting_;
    QueueCallback on_updated_;
public:
    //构造析构函数
    QueueService();
    ~QueueService() = default;

    //注册回调函数
    void setQueueCallback(QueueCallback callback){};

    //功能函数
    int in_queue(int order_id){};
    void advance_queue(){};
    bool is_Empty() const;
    bool is_too_long();

    //接口
    int getCurentEating(){};
    int getQueueID(QueueMsg msg){};
    std::vector<QueueMsg> getWaiting(){};

    //文件读写
    void loadFromFile(const std::string& file);
    void saveToFile(const std::string& file) const;
};
