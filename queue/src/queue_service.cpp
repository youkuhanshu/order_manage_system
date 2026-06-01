#include "include/queue_service.h"

//构造
QueueService::QueueService():current_eating(0),next_giving(1001){}

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
    waiting_.erase(waiting_.begin());
    current_eating++;

    if(on_updated_){
        on_updated_("advance_queue",&waiting_);
    }
}

bool QueueService::is_Empty() const{
    return waiting_.empty();
}

bool QueueService::is_too_long(){
    if(is_Empty()){
        return;
    }
    for(const auto& msg : waiting_){
        if(std::time(nullptr) - msg.in_time > time_threshold){
            return true;
        }
    }
    return false;
}

//接口
int QueueService::getCurentEating(){    
    return current_eating;
}
int QueueService::getQueueID(QueueMsg msg){
    return msg.queue_id;
}
std::vector<QueueMsg> QueueService::getWaiting(){
    return waiting_;
}

//文件读写
void QueueService::loadFromFile(const std::string& file){
    std::ifstream in(file);
    if(!in){
        return;
    }
    waiting_.clear();
    std::string line;
    while(std::getline(in,line)){
        if(line.empty()){
            continue;
        }
        waiting_.push_back(QueueMsg::from_String(line));
    }
    in.close();
}
void QueueService::saveToFile(const std::string& file) const{
    std::ofstream out(file);
    if(!out){
        return;
    }
    for(const auto& msg : waiting_){
        out << msg.to_String() << std::endl;  
    }
    out.close();
}

