#pragma once

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <QString>
#include <QList>

// 在C++中使用
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

class FileManager
{
private:
    const std::string MENU_FILE_PATH = "../storage/data/menu.txt";
    static std::vector<Dish> all_dishes_cpp;
    static QList<Dish_qt> all_dishes_qt;
    static QStringList all_categories_qt;
public:
    void LoadMenu();

    // 在C++中使用，获取菜单
    static std::vector<Dish> getMenu_cpp();

    // 在Qt中使用
    static QList<Dish_qt> getMenu_qt();

    // 在Qt中使用
    static QStringList getCategories_qt();
};
