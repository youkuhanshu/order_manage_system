#include "Order_mgr.hpp"

void OrderService::addDish(const Dish& dish) {
    order_.push_back(dish);
}

void OrderService::removeDish(const Dish& dish) {
    order_.erase(std::remove(order_.begin(), order_.end(), dish), order_.end());
}

double OrderService::calcCart() {
    double total = 0.0;
    for (const auto& dish : order_) {
        total += dish.price;
    }
    return total;
}

double OrderService::getDiscountRate(const Customer& customer) {
    return DISCOUNT_RATE(customer);
}

void OrderService::addToHistory(const Customer& customer) {
    order_history_.push_back(order_);
}

void OrderService::checkout(const Customer& customer) {
    total_price_ = calcCart() * getDiscountRate(customer);
    addToHistory(customer);
    order_.clear();
}

