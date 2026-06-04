#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "FileManager.h"

class OrderService{
public:
    OrderService() : order_(), order_history_(), total_price_(0.0) {}
    virtual ~OrderService() = default;

    virtual void addDish(const Dish& dish) = 0;
    virtual void removeDish(const Dish& dish) = 0;
    virtual double calcCart() = 0;
    virtual double checkout() = 0;
    virtual double getDiscountRate(const User& user_) = 0;
    virtual void updateUser(const User& user_, const std::vector<Dish>& order_, double total_price_) = 0;
    virtual static std::vector<std::vector<std::string>> loadUserHistoryOrders(const User& user_, const std::string& history_path) = 0;

private:
    std::vector<Dish> order_;
    std::vector<std::vector<Dish>> order_history_;

    double total_price_;
};


// calcCart(), checkout(), getDiscountRate(), cloneOrder(), addToHistory()