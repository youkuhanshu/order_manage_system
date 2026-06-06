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
    virtual double getDiscountRate(const User& user_);
    virtual void updateUser(const User& user_, const std::vector<Dish>& order_, double total_price_);
    static std::vector<std::vector<Dish>> loadUserHistoryOrders(const User& user_, const std::string& history_path);

private:
    User user_;
    std::vector<Dish> order_;
    std::vector<std::vector<Dish>> order_history_;
    double total_price_;
};