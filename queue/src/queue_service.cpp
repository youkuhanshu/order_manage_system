#include "queue_service.h"

//构造
QueueService::QueueService():current_calling(0),next_giving(1001){}

//析构：停止后台自动叫号线程
QueueService::~QueueService(){
    stop_auto_advance();
}

//注册回调函数
void QueueService::setQueueCallback(QueueCallback callback){
    on_updated_ = callback;
}

//功能函数
int QueueService::in_queue(int order_id){
    int id = next_giving++;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        waiting_.push_back(QueueMsg(id,order_id,std::time(nullptr)));
    }

    if(on_updated_){
        on_updated_("in_queue",&waiting_);
    }

    // 有新顾客入队，若自动叫号未运行则启动
    start_auto_advance();

    return id;
}

void QueueService::advance_queue(){
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if(waiting_.empty()){
        return;
    }
    taking_.push_back(*waiting_.begin());
    waiting_.erase(waiting_.begin());
    current_calling++;
}
// 注意: advance_queue() 不再触发回调，由 auto_advance_loop() 统一在锁外触发

bool QueueService::is_Empty() const{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return waiting_.empty();
}

bool QueueService::is_too_long(){
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if(waiting_.empty()){
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
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return current_calling;
}
int QueueService::getQueueID(QueueMsg msg){
    return msg.queue_id;
}
std::vector<QueueMsg> QueueService::getWaiting(){
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return waiting_;
}
std::vector<QueueMsg> QueueService::getTaking(){
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return taking_;
}

// ================================================================
//  自动叫号（后台线程，随机时间间隔，按序叫号）
// ================================================================

void QueueService::start_auto_advance(){
    if (auto_advance_running_) return;
    auto_advance_running_ = true;
    auto_advance_thread_ = std::thread(&QueueService::auto_advance_loop, this);
}

void QueueService::stop_auto_advance(){
    auto_advance_running_ = false;
    if (auto_advance_thread_.joinable()) {
        auto_advance_thread_.join();
    }
}

void QueueService::auto_advance_loop(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(5, 20);  // 5~20 秒随机间隔

    while (auto_advance_running_) {
        // 随机休眠（每秒检查一次停止标志）
        int sleep_sec = dist(gen);
        for (int i = 0; i < sleep_sec && auto_advance_running_; i++) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (!auto_advance_running_) break;

        // 加锁、叫号、解锁
        bool advanced = false;
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (!waiting_.empty()) {
                taking_.push_back(*waiting_.begin());
                waiting_.erase(waiting_.begin());
                current_calling++;
                advanced = true;
            }
        }

        // 在锁外触发回调（避免死锁）
        if (advanced && on_updated_) {
            on_updated_("advance_queue", &waiting_);
        }
    }
}
