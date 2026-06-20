#include "Order_mgr.hpp"

#include <algorithm>

void OrderService::addDish(const Dish& dish) {
    order_.push_back(dish);
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
    if (user.level == "SILVER") {
        return 0.95;
    }
    if (user.level == "GOLD") {
        return 0.85;
    }
    if (user.level == "PLATINUM") {
        return 0.75;
    }
    return 1.0;
}

double OrderService::checkout(FileManager& fileManager) {
    total_price_ = calcCart() * getDiscountRate(user_);
    user_.total_spent += total_price_;

    if (user_.total_spent >= 10000.0) {
        user_.level = "PLATINUM";
    } else if (user_.total_spent >= 5000.0) {
        user_.level = "GOLD";
    } else if (user_.total_spent >= 1000.0) {
        user_.level = "SILVER";
    } else {
        user_.level = "REGULAR";
    }

    fileManager.SaveCheckout(user_, order_);
    return total_price_;
}

void OrderService::removeOneDish(int dishId) {
    for (auto it = order_.begin(); it != order_.end(); ++it) {
        if (it->id == dishId) {
            order_.erase(it);
            break;
        }
    }
}
