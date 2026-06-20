#include"Comment_service.h"
#include"algorithm"

// 静态成员定义（全局菜品平均分排名索引）
std::vector<std::string> DishComment_msg::all_dish_rate_rank;

    //注册回调函数
    void CommentService::setCommentCallback(CommentCallback callback){
        on_updated_ = callback;
    };

    //功能函数
    void CommentService::AddComment(CommentMsg msg){
        All_Comments_.push_back(msg);
        comment_num_++;

        //自动更新，必须先更新菜品平均分再更新全部菜品评分排行
        UpdateCommentRank(msg);
        UpdateDishComment(msg);
        UpdateAllDishRank(msg);

        if(on_updated_){
            on_updated_("AddComment",&All_Comments_);
        }
    }
    
    void CommentService::UpdateAllDishRank(CommentMsg msg) {
        for (const auto& dish_id : msg.dish_ids) {
            // 1. 从排名列表中移除该菜品的旧条目
            auto& rank_list = DishComment_msg::all_dish_rate_rank;
            rank_list.erase(std::remove(rank_list.begin(), rank_list.end(), dish_id), rank_list.end());
            // 2. 根据更新后的平均分，找到新位置并插入（确保从高分到低分排序）
            if (Dish_Comments_.find(dish_id) == Dish_Comments_.end()) {
                continue; 
            }
            double current_dish_rate = Dish_Comments_.at(dish_id).aver_rate;
            bool is_inserted = false;
            for(auto it = rank_list.begin(); it != rank_list.end(); ++it) {
                if (Dish_Comments_.at(*it).aver_rate <= current_dish_rate) {
                    rank_list.insert(it, dish_id); // 在第一个不高于当前分的位置插入
                    is_inserted = true;
                    break;
                }
            }
            if (!is_inserted) {
                rank_list.push_back(dish_id); // 分数最低，插到末尾
            }
        }
    }
    

    void CommentService::UpdateCommentRank(CommentMsg msg){
         // 空列表时直接追加（第一条评论）
         if (rate_rank_.empty()) {
             rate_rank_.push_back(comment_num_);
             return;
         }
         for(auto it = rate_rank_.begin();it != rate_rank_.end();it++){
            if(All_Comments_[*it].rate > msg.rate){
                continue;
            }
            rate_rank_.insert(it,comment_num_);
            return;
        }
        // 新评论评分最低，插入末尾
        rate_rank_.push_back(comment_num_);
    }

    void CommentService::UpdateDishComment(CommentMsg msg){
        for(auto it = msg.dish_ids.begin();it != msg.dish_ids.end();it++){
            std::string dish_name = *it;
            Dish_Comments_[dish_name].comments.push_back(msg);
            Dish_Comments_[dish_name].aver_rate =
            (Dish_Comments_[dish_name].aver_rate * Dish_Comments_[dish_name].dish_comment_num + msg.rate)
            / (Dish_Comments_[dish_name].dish_comment_num + 1);

            Dish_Comments_[dish_name].dish_comment_num++;
        }
    }

    //接口
    std::vector<CommentMsg> CommentService::getComment(std::string rank_type){
        std::vector<CommentMsg> com;
        if(rank_type == "time"){
            com = All_Comments_;
        }
        else if(rank_type == "rate"){
            com.clear();
            for(auto it = rate_rank_.begin();it != rate_rank_.end();it++){
                com.push_back(All_Comments_[*it]);
            }
        }
        return com;
    }
    double CommentService::getDishAverRate(const std::string& dish_name){
        return Dish_Comments_[dish_name].aver_rate;
    }
    std::vector<CommentMsg> CommentService::getDishComments(const std::string& dish_name,std::string rank_type){
        DishComment_msg dish_msg = Dish_Comments_[dish_name];
        std::vector<CommentMsg> com;
        if(rank_type == "time"){
            com = dish_msg.comments;
        }
        else if(rank_type == "rate"){
            com.clear();
            for(auto it = dish_msg.rate_rank.begin();it != dish_msg.rate_rank.end();it++){
                com.push_back(dish_msg.comments[*it]);
            }
        }
        return com;
    }

    std::vector<DishComment_msg> CommentService::getBest5Dishs() {
        std::vector<DishComment_msg> best_dishes;
        const auto& rank_list = DishComment_msg::all_dish_rate_rank;

        if (rank_list.empty()) {
            return best_dishes;
        }

        // 1. 填充五个
        int count = 0;
        for (const auto& dish_id : rank_list) {
            if (count >= 5) {
                break;
            }
            // 安全检查：确保 dish_id 存在于 Dish_Comments_ 中
            if (Dish_Comments_.count(dish_id)) {
                best_dishes.push_back(Dish_Comments_.at(dish_id));
                count++;
            }
        }

        // 2. 多于五个，检查并列
        if (rank_list.size() > 5 && ! best_dishes.empty()) {
        double fifth_rate = best_dishes.back().aver_rate; // 第5名的分数
        for (int i = 5; i < rank_list.size(); ++i) {
            const auto& dish_id = rank_list[i];
            // 安全检查：确保 dish_id 存在，且分数与第五名相同
            if (Dish_Comments_.count(dish_id) && Dish_Comments_.at(dish_id).aver_rate == fifth_rate) {
                best_dishes.push_back(Dish_Comments_.at(dish_id));
            } 
            else {
                break;
            }
        }
    }
    return best_dishes;
}