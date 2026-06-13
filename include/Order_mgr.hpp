#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "FileManager.h"

class OrderService{
public:
    OrderService(const User& user) : user_(user), order_history_(), total_price_(0.0) {}
    virtual ~OrderService() = default;

    virtual void addDish(const Dish& dish);
    virtual void removeDish(const Dish& dish);
    virtual double calcCart();
    virtual double checkout();
    void setUser(const User& user) { user_ = user; }  // 用户等级变化后同步更新
    virtual double getDiscountRate(const User& user_);
    virtual void updateUser(const User& user_, const std::vector<Dish>& order_, double total_price_);
    static std::vector<std::vector<Dish>> loadUserHistoryOrders(const User& user_, const std::string& history_path);

    // 购物车辅助
    const std::vector<Dish>& getOrder() const { return order_; }
    void clearOrder() { order_.clear(); total_price_ = 0.0; }
    void removeOneDish(int dishId);   // 从 order_ 中移除第一个匹配的菜品

private:
    User user_;
    std::vector<Dish> order_;
    std::vector<std::vector<Dish>> order_history_;
    double total_price_;
};