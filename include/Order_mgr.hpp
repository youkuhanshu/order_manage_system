#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "FileManager.h"

class OrderService{
public:
    OrderService(const User& user) : user_(user), total_price_(0.0) {}
    virtual ~OrderService() = default;

    virtual void addDish(const Dish& dish);
    virtual void removeDish(const Dish& dish);
    virtual double calcCart();
    virtual double checkout(FileManager& fileManager);
    void setUser(const User& user) { user_ = user; }  // 用户等级变化后同步更新
    const User& getUser() const { return user_; }
    virtual double getDiscountRate(const User& user_);

    // 购物车辅助
    const std::vector<Dish>& getOrder() const { return order_; }
    void clearOrder() { order_.clear(); total_price_ = 0.0; }
    void removeOneDish(int dishId);   // 从 order_ 中移除第一个匹配的菜品

private:
    User user_;
    std::vector<Dish> order_;
    double total_price_;
};
