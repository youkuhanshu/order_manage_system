#pragma once
#include <iostream>
#include <string>
#include <vector>
#include"queue_msg.hpp"
#include <functional>

using QueueCallback = std::function<void(const std::string& type,const void* data)>;
#define time_threshold 10 //单位是秒

class QueueService{
private:
    int current_calling;                  //现在叫到号数
    int next_giving;                     //下一个应给号数
    std::vector<QueueMsg> waiting_;      //排队队列
    std::vector<QueueMsg> taking_;       //取餐队列
public:
    //构造析构函数
    QueueService();
    ~QueueService() = default;


    //功能函数
    int in_queue(int order_id);     //加入排队队列
    void advance_queue();           //叫号：预约队列队首 → 取餐队列
    void take_meal(int queue_id);   //顾客取餐：把该号移出取餐队列
    bool is_Empty() const;
    bool is_too_long();             //等待时间是否超过阈值
    int nextAdvanceDelay() const; // 返回 5~20 秒随机间隔，供上层定时器使用

    //接口
    int getCurentCall();
    int getQueueID(QueueMsg msg);
    std::vector<QueueMsg> getWaiting();
    std::vector<QueueMsg> getTaking();

};
