#include "FileManager.h"

std::vector<Dish> FileManager::all_dishes_cpp;
QList<Dish_qt> FileManager::all_dishes_qt;
QStringList FileManager::all_categories_qt;
QList<Dish_qt> FileManager::recommend_dishes_qt;

void FileManager::LoadMenu() {
    all_dishes_cpp.clear();
    all_dishes_qt.clear();
    all_categories_qt.clear();

    std::string line;

    std::ifstream ifs;
    ifs.open(MENU_FILE_PATH, std::ios::in);

    if ( !ifs.is_open() ) {
        std::cout << "无法打开菜单文件" << std::endl;
        return;
    }

    while (getline(ifs, line)) { // 从文件流中读取一整行文本，直到遇到换行符或文件结束
        Dish d;
        Dish_qt d_qt;
        std::stringstream ss(line);

        // 将菜品填入vector
        ss >> d.id >> d.name >> d.price >> d.description >> d.sales >> d.rating >> d.category;
        all_dishes_cpp.push_back(d);

        // 将菜品填入QList
        d_qt.id = d.id;
        d_qt.name = QString::fromStdString(d.name);
        d_qt.price = d.price;
        d_qt.description = QString::fromStdString(d.description);
        d_qt.sales = d.sales;
        d_qt.rating = d.rating;
        d_qt.category = QString::fromStdString(d.category);
        all_dishes_qt.append(d_qt);

        // 获取菜品种类，用于分类栏
        if (!all_categories_qt.contains(d_qt.category)) {
            all_categories_qt.append(d_qt.category);
        }
    }

    for (size_t i = 0;i < 5;i++) {
        recommend_dishes_qt.append(all_dishes_qt[i]);
    }

    ifs.close();
}

// 在C++中使用
std::vector<Dish> FileManager::getMenu_cpp() {
    return all_dishes_cpp;
}

// 在Qt中使用
QList<Dish_qt> FileManager::getMenu_qt() {
    return all_dishes_qt;
}

// 在Qt中使用
QStringList FileManager::getCategories_qt() {
    return all_categories_qt;
}

// 在Qt中使用
QList<Dish_qt> FileManager::getRecommend_qt() {
    return recommend_dishes_qt;
}
