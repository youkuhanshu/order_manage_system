#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <QString>
#include <QList>
#include <QStringList>
#include <QDateTime>

#include "queue_msg.hpp"
#include "Comment_msg.hpp"

// 前置声明（toQt / toCpp 互相引用，需要提前声明目标类型）
struct Dish_qt;
struct User_qt;
struct QueueMsg_qt;
struct CommentMsg_qt;
struct DishComment_msg_qt;

// 菜品

// C++ 版，供后端逻辑使用
struct Dish
{
    int id;
    std::string name;
    double price;
    std::string description;
    int sales;
    double rating;
    std::string category;
    int comment_count;

    // 为std::remove提供Dish的==运算符
    bool operator==(const Dish& other) const {
        return id == other.id;
    }
};

// Qt 版，供 UI 使用
struct Dish_qt
{
    int id;
    QString name;
    double price;
    QString description;
    int sales;
    double rating;
    QString category;
    int comment_count;
};

// 用户

// C++ 版，供后端逻辑使用
struct User
{
    int id;
    std::string name;
    std::string password;
    std::string level; // "REGULAR" | "SILVER" | "GOLD" | "PLATINUM"
    double total_spent;
};

// Qt 版，供 UI 使用
struct User_qt
{
    int id;
    QString name;
    QString password;
    QString level; // "REGULAR" | "SILVER" | "GOLD" | "PLATINUM"
    double total_spent;
};

// 评论消息

// Qt 版，供 UI 使用
struct CommentMsg_qt
{
    QString user_id;
    QStringList dish_ids;  // 支持多菜品评论
    QString comment;
    int rate;
    QDateTime in_time;
};

// Qt 版，供 UI 使用
struct DishComment_msg_qt
{
    double aver_rate;         // 该菜品平均分
    int dish_comment_num;  // 该菜品评论数
    QList<CommentMsg_qt> comments;          // 该菜品对应评论
    QList<int> rate_rank;         // 评论分数排列索引
    static QStringList all_dish_rate_rank; // 全菜品平均分排名索引
};

class CommentService; // 前向声明，用于复用评分更新逻辑

// FileManager —— 文件读写
class FileManager
{
private:
    const std::string MENU_FILE_PATH = "../storage/data/menu.txt";
    const std::string QUEUE_FILE_PATH = "../storage/data/queue.txt";
    const std::string COMMENT_FILE_PATH = "../storage/data/comment.txt";
    const std::string USER_FILE_PATH = "../storage/data/users.txt";
    const std::string HISTORY_ORDER_FILE_PATH = "../storage/data/history_order.txt";

    static std::vector<Dish> all_dishes_cpp;
    static QList<Dish_qt> all_dishes_qt;
    static QStringList all_categories_qt;
    static QList<Dish_qt> recommend_by_sales;
    static QList<Dish_qt> recommend_by_rating;
    static QList<Dish_qt> recommend_by_comments;

    static std::vector<User> all_users_cpp;
    static std::vector<QueueMsg> all_queue_;
    static std::vector<CommentMsg> all_comments_;

public:
    // 菜单
    void LoadMenu();
    static std::vector<Dish> getMenu_cpp();
    static QList<Dish_qt> getMenu_qt();
    static QStringList getCategories_qt();
    static QList<Dish_qt> getRecommendBySales();
    static QList<Dish_qt> getRecommendByRating();
    static QList<Dish_qt> getRecommendByComments();

    // 用户
    void LoadUsers();
    static std::vector<User> getUsers_cpp();
    void addUser(int id, std::string name, std::string password);
    void SaveUser(const User& user);
    std::vector<std::vector<Dish>> LoadUserHistoryOrders(const User& user) const;
    void SaveCheckout(const User& user, const std::vector<Dish>& order);

    // 排队
    void LoadQueue();
    void SaveQueue(const std::vector<QueueMsg>& queue) const;
    static std::vector<QueueMsg> getQueue();

    // 评论
    void LoadComments();
    void SaveComments(const std::vector<CommentMsg>& comments) const;
    void AddCommentAndUpdateMenu(const CommentMsg& comment, CommentService& cs);
    static std::vector<CommentMsg> getComments();

    // 类型转换
    User_qt user_to_qt(const User& user_cpp);
    User user_to_cpp(const User_qt& user_qt);
    Dish_qt dish_to_qt(Dish& dish_cpp);
    Dish dish_to_cpp(const Dish_qt& dish_qt);
    QueueMsg queuemsg_to_cpp(const QueueMsg_qt& q);
    QueueMsg_qt queuemsg_to_qt(const QueueMsg& msg);
    CommentMsg commentmsg_to_cpp(const CommentMsg_qt& q);
    CommentMsg_qt commentmsg_to_qt(const CommentMsg& msg);
    DishComment_msg dishcomment_to_cpp(const DishComment_msg_qt& q);
    DishComment_msg_qt dishcomment_to_qt(const DishComment_msg& msg);
};
