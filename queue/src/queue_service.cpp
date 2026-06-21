#include "queue_service.h"
#include <cstdlib>

//构造
QueueService::QueueService():current_calling(0),next_giving(1001){}


//功能函数
int QueueService::in_queue(int order_id){
    int id = next_giving++;
    waiting_.push_back(QueueMsg(id,order_id,std::time(nullptr)));

    return id;
}

void QueueService::advance_queue(){
    if(is_Empty()){
        return;
    }
    taking_.push_back(*waiting_.begin());
    waiting_.erase(waiting_.begin());
    current_calling++;

}

//顾客取餐：从取餐队列里移除该号
void QueueService::take_meal(int queue_id){
    for(auto it = taking_.begin(); it != taking_.end(); ++it){
        if(it->queue_id == queue_id){
            taking_.erase(it);
            break;
        }
    }
}

bool QueueService::is_Empty() const{
    return waiting_.empty();
}

bool QueueService::is_too_long(){
    if(is_Empty()){
        return false;
    }
    for(const auto& msg : waiting_){
        if(std::time(nullptr) - msg.in_time > time_threshold){
            return true;
        }
    }
    return false;
}

// 返回 5~20 秒随机叫号间隔
int QueueService::nextAdvanceDelay() const {
    return 5 + std::rand() % 16;
}

//接口
int QueueService::getCurentCall(){
    return current_calling;
}
int QueueService::getQueueID(QueueMsg msg){
    return msg.queue_id;
}
std::vector<QueueMsg> QueueService::getWaiting(){
    return waiting_;
}
std::vector<QueueMsg> QueueService::getTaking(){
    return taking_;
}
