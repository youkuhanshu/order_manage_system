#pragma once
#include <iostream>
#include <ctime>

struct QueueMsg{
int queue_id;               //拿到的队列号
int order_id;               //对应的订单号
std::time_t in_time;        //时间戳

//构造
QueueMsg():queue_id(0),order_id(0),in_time(0){}
QueueMsg(int queuei,int orderi,std::time_t intim):queue_id(queuei),order_id(orderi),in_time(intim){}

//string和Queue_msg类型转换函数
std::string to_String() const{
    return std::to_string(queue_id) + "|" +
           std::to_string(order_id) + "|" +
           std::to_string(in_time);
}
static QueueMsg from_String(const std::string& str){
    QueueMsg msg;
    //记录分界点位置
    size_t p1 = str.find('|');
    size_t p2 = str.find('|',p1+1);
    //若成功则填充
    if(p1 != std::string::npos && p2 != std::string::npos){
        msg.queue_id = std::stoi(str.substr(0,p1));
        msg.order_id = std::stoi(str.substr(p1 + 1,p2 - p1 -1));
        //时间戳用long long更安全
        msg.in_time = std::stoll(str.substr(p2 + 1));
        return msg;
    }
    //等着加个失败管理
    return msg;
}
};

struct QueueMsg_qt
{
    int queue_id;  // 队列号
    int order_id;  // 订单号
    QDateTime in_time;   // 入队时间
};