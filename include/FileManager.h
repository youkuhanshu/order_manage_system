#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <QString>
#include <QList>

#include "queue_msg.hpp"
#include "Comment_msg.hpp"

// 在C++中使用，菜品
struct Dish
{
    int id;          // 编号
    std::string name;       // 菜名
    double price;       // 价格（元）
    std::string description; // 说明
    int sales;       // 销量
    double rating;      // 平均评分
    std::string category;   // 分类
};

// 在Qt中使用
struct Dish_qt
{
    int id;          // 编号
    QString name;       // 菜名
    double price;       // 价格（元）
    QString description; // 说明
    int sales;       // 销量
    double rating;      // 平均评分
    QString category;   // 分类
};

// 在C++中使用，用户
struct User
{
    int id;
    std::string name;
    std::string password;
    int level;
};

class FileManager
{
private:
    const std::string MENU_FILE_PATH = "../storage/data/menu.txt";
    const std::string QUEUE_FILE_PATH = "../storage/data/queue.txt";
    const std::string COMMENT_FILE_PATH = "../storage/data/comment.txt";
    const std::string USER_FILE_PATH = "../storage/data/users.txt";

    static std::vector<Dish> all_dishes_cpp;
    static QList<Dish_qt> all_dishes_qt;
    static QStringList all_categories_qt;
    static QList<Dish_qt> recommend_dishes_qt;

    static std::vector<User> all_users_cpp;

    static std::vector<QueueMsg> all_queue_;
    static std::vector<CommentMsg> all_comments_;

public:
    // 菜单
    void LoadMenu();
    static std::vector<Dish> getMenu_cpp();
    static QList<Dish_qt> getMenu_qt();
    static QStringList getCategories_qt();
    static QList<Dish_qt> getRecommend_qt();

    // 用户
    void LoadUsers();
    static std::vector<User> getUsers_cpp();
    void addUser(int id, std::string name, std::string password);

    // 排队
    void LoadQueue();
    void SaveQueue(const std::vector<QueueMsg>& queue) const;
    static std::vector<QueueMsg> getQueue();

    // 评论
    void LoadComments();
    void SaveComments(const std::vector<CommentMsg>& comments) const;
    static std::vector<CommentMsg> getComments();
};
