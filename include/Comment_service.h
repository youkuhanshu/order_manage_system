#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include"Comment_msg.hpp"
#include <functional>
#include <fstream>

using CommentCallback = std::function<void(const std::string& type,const void* data)>;
class CommentService{
private:
    std::vector<CommentMsg> All_Comments_;                     //时间排列全部评论
    std::vector<int> rate_rank_;                               //分数排列评论索引
    std::map<std::string,DishComment_msg> Dish_Comments_;      //菜品评论地图
    int comment_num_=0;                                        //全部评论数
    CommentCallback on_updated_;                               //回调函数         

public:
    //构造析构函数
    CommentService() = default;
    ~CommentService() = default;

    //注册回调函数
    void setCommentCallback(CommentCallback callback){};

    //功能函数
    void AddComment(CommentMsg msg){};                          //添加评论
    void UpdateCommentRank(CommentMsg msg){};                   //更新评论分数排名索引
    void UpdateDishComment(CommentMsg msg){};                   //更新菜品评论和菜品分数排名索引

    //接口
    std::vector<CommentMsg> getComment(std::string rank_type){};
    double GetDishAverRate(const std::string& dish_name);
    std::vector<CommentMsg> GetDishComments(const std::string& dish_name,std::string rank_type);

    //文件读写
    void loadFromFile(const std::string& file);
    void saveToFile(const std::string& file) const;
};