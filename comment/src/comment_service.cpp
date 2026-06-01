#include"Comment_service.h"

    
    //注册回调函数
    void CommentService::setCommentCallback(CommentCallback callback){
        on_updated_ = callback;
    };

    //功能函数
    void CommentService::AddComment(CommentMsg msg){
        All_Comments_.push_back(msg);
        comment_num_++;

        UpdateCommentRank(msg);

        if(on_updated_){
            on_updated_("AddComment",&All_Comments_);
        }
    }

    void CommentService::UpdateCommentRank(CommentMsg msg){
         for(auto it = rate_rank_.begin();it != rate_rank_.end();it++){
            if(All_Comments_[*it].rate > msg.rate){
                continue;
            }
            rate_rank_.insert(it,comment_num_);
            break;
        }
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
        if(on_updated_){
            on_updated_("UpdateDishComment",&Dish_Comments_);
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
    double CommentService::GetDishAverRate(const std::string& dish_name){
        return Dish_Comments_[dish_name].aver_rate;
    }
    std::vector<CommentMsg> CommentService::GetDishComments(const std::string& dish_name,std::string rank_type){
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

    //文件读写
    void CommentService::loadFromFile(const std::string& file){
        std::ifstream in(file);
        if(!in){
            return;
        }
        All_Comments_.clear();
        std::string line;
        while(std::getline(in,line)){
            if(line.empty()){
                continue;
            }
            All_Comments_.push_back(CommentMsg::from_String(line));
            }
        in.close();
    }
    void CommentService::saveToFile(const std::string& file) const{
         std::ofstream out(file);
        if(!out){
            return;
        }
        for(const auto& msg : All_Comments_){
            out << msg.to_String() << std::endl;  
        }
        out.close();    
    }