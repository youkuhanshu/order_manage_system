#include "Order_mgr.hpp"

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <sstream>

static std::vector<std::vector<Dish>> loadUserHistoryOrders(const User& user, const std::string& history_path) {
    std::ifstream infile(history_path);
    if (!infile.is_open()) {
        std::cerr << "Failed to open " << history_path << std::endl;
        return {};
    }

    std::vector<std::vector<Dish>> orders;
    std::string line;
    while (std::getline(infile, line)) {
        std::vector<Dish> order;

        if (line.empty() || (!line.empty() && line[0] == '#')) continue;

        std::istringstream iss(line);
        std::string name;
        if (!(iss >> name)) continue;
        if (name != user.name) continue;

        std::string dish;
        while (iss >> dish) {
            // Dish 目前是 plain struct（无接收 std::string 的构造函数），这里先只填菜名
            Dish d{};
            d.name = dish;
            order.push_back(d);
        }
        orders.push_back(order);
    }
    return orders;
}

void OrderService::addDish(const Dish& dish) {
    order_.push_back(dish);
    //这里尚不知道怎么在ui界面中调用这个函数来加菜，（应当要多次例如在ui中点击加菜按钮时调用填充进order_里）
}

void OrderService::removeDish(const Dish& dish) {
    order_.erase(std::remove(order_.begin(), order_.end(), dish), order_.end());
}

double OrderService::calcCart() {
    total_price_ = 0.0;
    for (const auto& dish : order_) {
        total_price_ += dish.price;
    }
    return total_price_;
}

double OrderService::getDiscountRate(const User& user) {
    if (user.level == "REGULAR") {
        return 1.0;
    } 
    else if (user.level == "SILVER") {
        return 0.95;
    } 
    else if (user.level == "GOLD") {
        return 0.85;
    } 
    else if (user.level == "PLATINUM") {
        return 0.75;
    }
    return 1.0;
}

void OrderService::updateUser(const User& user_, const std::vector<Dish>& order_, double total_price_) {
    // ---- 1) 写入 history_orders.txt：用户名 + 本次订单菜名（空格分隔）----
    const std::string history_path = "../storage/data/history_orders.txt";
    {
        std::ostringstream order_line;
        order_line << user_.name;
        for (const auto& dish : order_) {
            order_line << " " << dish.name;
        }

        std::ofstream outfile(history_path, std::ios::app);
        if (!outfile.is_open()) {
            std::cerr << "Failed to open " << history_path << " for writing" << std::endl;
            return;
        }
        outfile << order_line.str() << "\n";
    }

    // ---- 2) 更新 menu.txt：order_ 中出现的菜名销量 +1（重复出现要重复+1）----
    const std::string menu_path = "../storage/data/menu.txt";
    std::unordered_map<std::string, int> dish_counts;
    for (const auto& dish : order_) {
        dish_counts[dish.name] += 1;
    }

    {
        std::ifstream in(menu_path);
        if (!in.is_open()) {
            std::cerr << "Failed to open " << menu_path << " for reading" << std::endl;
            return;
        }

        std::vector<std::string> out_lines;
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || (!line.empty() && line[0] == '#')) {
                out_lines.push_back(line);
                continue;
            }

            Dish d{};
            std::stringstream ss(line);
            // menu.txt: id 菜名 价格 描述 销量 平均得分 类型 评论个数
            ss >> d.id >> d.name >> d.price >> d.description >> d.sales >> d.rating >> d.category >> d.comment_count;

            auto it = dish_counts.find(d.name);
            if (it != dish_counts.end()) {
                d.sales += it->second;
            }

            std::ostringstream rebuilt;
            rebuilt << d.id << " " << d.name << " " << d.price << " " << d.description << " "
                    << d.sales << " " << d.rating << " " << d.category << " " << d.comment_count;
            out_lines.push_back(rebuilt.str());
        }
        in.close();

        std::ofstream out(menu_path, std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "Failed to open " << menu_path << " for writing" << std::endl;
            return;
        }
        for (const auto& l : out_lines) {
            out << l << "\n";
        }
    }

    // ---- 3) 更新 users.txt：找到用户，更新会员等级，并把总消费加上 total_price_ ----
    const std::string users_path = "../storage/data/users.txt";
    {
        std::ifstream in(users_path);
        if (!in.is_open()) {
            std::cerr << "Failed to open " << users_path << " for reading" << std::endl;
            return;
        }

        std::vector<std::string> out_lines;
        std::string line;
        while (std::getline(in, line)) {
            if (line.empty() || (!line.empty() && line[0] == '#')) {
                out_lines.push_back(line);
                continue;
            }

            std::stringstream ss(line);
            int id = 0;
            std::string name;
            std::string password;
            std::string level;
            double total_spent = 0.0;

            if (!(ss >> id >> name >> password >> level)) {
                out_lines.push_back(line);
                continue;
            }
            if (!(ss >> total_spent)) {
                total_spent = 0.0;
            }

            if (name == user_.name) {
                total_spent += total_price_;

                if (total_spent >= 10000.0) level = "PLATINUM";
                else if (total_spent >= 5000.0) level = "GOLD";
                else if (total_spent >= 1000.0) level = "SILVER";
                else level = "REGULAR";
            }

            std::ostringstream rebuilt;
            rebuilt << id << " " << name << " " << password << " " << level << " " << total_spent;
            out_lines.push_back(rebuilt.str());
        }
        in.close();

        std::ofstream out(users_path, std::ios::trunc);
        if (!out.is_open()) {
            std::cerr << "Failed to open " << users_path << " for writing" << std::endl;
            return;
        }
        for (const auto& l : out_lines) {
            out << l << "\n";
        }
    }
}

double OrderService::checkout() {  
    total_price_ = calcCart() * getDiscountRate(user);
    updateUser(user, order_,total_price_);

    return total_price_;
}


#include "Order_mgr.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

static std::vector<std::vector<std::string>> loadUserHistoryOrders(const User& user, const std::string& history_path) {
    std::ifstream infile(history_path);
    if (!infile.is_open()) {
        std::cerr << "Failed to open " << history_path << std::endl;
        return {};
    }

    std::vector<std::vector<std::string>> orders;
    std::string line;
    while (std::getline(infile, line)) {
        order.clear();
        if (line.empty() || (!line.empty() && line[0] == '#')) continue;

        std::istringstream iss(line);
        std::string name;
        if (!(iss >> name)) continue;
        if (name != user.name) continue;

        std::string dish;
        while (iss >> dish) {
            order_.push_back(dish);
        }
        //每一次运行到这里都是加载了一个历史订单，现在需要在ui中一行一行显示出来,并且可以直接按+号加入购物车
    }

}

void OrderService::addDish(const Dish& dish) {
    order_.push_back(dish);
    //这里尚不知道怎么在ui界面中调用这个函数来加菜，（应当要多次例如在ui中点击加菜按钮时调用填充进order_里）
}

void OrderService::removeDish(const Dish& dish) {
    order_.erase(std::remove(order_.begin(), order_.end(), dish), order_.end());
}

double OrderService::calcCart() {
    total_price_ = 0.0;
    for (const auto& dish : order_) {
        total_price_ += dish.price;
    }
    return total_price_;
}

double OrderService::getDiscountRate(const User& user) {
    if (user.level == "REGULAR") {
        return 1.0;
    } 
    else if (user.level == "SILVER") {
        return 0.95;
    } 
    else if (user.level == "GOLD") {
        return 0.85;
    } 
    else if (user.level == "PLATINUM") {
        return 0.75;
    }
    return 1.0;
}

void OrderService::updateUser(const User& user_, const std::vector<Dish>& order_, double total_price_) {
    User updated_user = user_;
    updated_user.sum_spent += static_cast<double>(total_price_);

    if (updated_user.sum_spent >= 10000) {
        updated_user.level = "PLATINUM";
    } 
    else if (updated_user.sum_spent >= 5000) {
        updated_user.level = "GOLD";
    } 
    else if (updated_user.sum_spent >= 1000) {
        updated_user.level = "SILVER";
    } 
    else {
        updated_user.level = "REGULAR";
    }

    const std::string history_path = "../storage/data/history_order.txt";

    std::ostringstream order_line;
    order_line << updated_user.name;
    for (const auto& dish : order_) {
        order_line << " " << dish.name;
    }

    std::ofstream outfile(history_path, std::ios::app);
    if (!outfile.is_open()) {
        std::cerr << "Failed to open " << history_path << " for writing" << std::endl;
        return;
    }
    outfile << order_line.str() << "\n";
    // FileManager fileManager;
    // fileManager.updateUser(updated_user);
    //这里要写一个文件读写，更新用户信息（会员等级，**历史订单**,还有菜本身被点了一次（更新菜的销量））到文件中
}

double OrderService::checkout() {  
    total_price_ = calcCart() * getDiscountRate(user);
    updateUser(user, order_,total_price_);

    return total_price_;
}

