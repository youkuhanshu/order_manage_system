#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>
#include <chrono>
#include"queue_msg.hpp"
#include <functional>

using QueueCallback = std::function<void(const std::string& type,const void* data)>;
#define time_threshold 6000 //单位是秒

class QueueService{
private:
    int current_calling;                  //现在叫到号数
    int next_giving;                     //下一个应给号数
    std::vector<QueueMsg> waiting_;      //排队队列
    std::vector<QueueMsg> taking_;       //取餐队列
    QueueCallback on_updated_;           //回调函数

    // 自动叫号（随机时间间隔，后台线程）
    std::thread auto_advance_thread_;
    std::atomic<bool> auto_advance_running_{false};
    mutable std::mutex queue_mutex_;
    void auto_advance_loop();            // 后台线程函数

public:
    //构造析构函数
    QueueService();
    ~QueueService();

    //注册回调函数
    void setQueueCallback(QueueCallback callback);

    //功能函数
    int in_queue(int order_id);     //加入排队队列
    void advance_queue();           //有人取到餐
    bool is_Empty() const;
    bool is_too_long();             //等待时间是否超过阈值

    //自动叫号控制
    void start_auto_advance();      //启动后台自动叫号
    void stop_auto_advance();       //停止后台自动叫号

    //接口
    int getCurentCall();
    int getQueueID(QueueMsg msg);
    std::vector<QueueMsg> getWaiting();
    std::vector<QueueMsg> getTaking();

};
