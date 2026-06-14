#include "queue_service.h"

//构造
QueueService::QueueService():current_calling(0),next_giving(1001){}

//注册回调函数
void QueueService::setQueueCallback(QueueCallback callback){
    on_updated_ = callback;
}

//功能函数
int QueueService::in_queue(int order_id){
    int id = next_giving++;
    waiting_.push_back(QueueMsg(id,order_id,std::time(nullptr)));

    if(on_updated_){
        on_updated_("in_queue",&waiting_);
    }

    return id;
}

void QueueService::advance_queue(){
    if(is_Empty()){
        return;
    }
    taking_.push_back(*waiting_.begin());
    waiting_.erase(waiting_.begin());
    current_calling++;

    if(on_updated_){
        on_updated_("advance_queue",&waiting_);
    }
}

void QueueService::pickup_meal(int queue_id){
    for (auto it = taking_.begin(); it != taking_.end(); ++it) {
        if (it->queue_id == queue_id) {
            taking_.erase(it);
            if (on_updated_) {
                on_updated_("pickup_meal", &taking_);
            }
            return;
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
