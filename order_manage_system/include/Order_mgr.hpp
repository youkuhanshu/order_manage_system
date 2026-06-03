#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "FileManager.h"

#define DISCOUNT_RATE(customer) (customer.getMembership() == Customer::Membership::REGULAR ? 1.0 : \
                                customer.getMembership() == Customer::Membership::SILVER ? 0.95 : \
                                customer.getMembership() == Customer::Membership::GOLD ? 0.85 : 0.75)

class OrderService{
public:
    virtual void addDish(const Dish& dish) = 0;
    virtual void removeDish(const Dish& dish) = 0;
    virtual double calcCart() = 0;
    virtual void checkout(const Customer& customer) = 0;
    virtual double getDiscountRate(const Customer& customer) = 0;
    virtual OrderService* cloneOrder() = 0;
    virtual void addToHistory(const Customer& customer) = 0;

private:
    std::vector<Dish> order_;
    std::vector<std::vector<Dish>> order_history_;

    double total_price_;
    

};


// calcCart(), checkout(), getDiscountRate(), cloneOrder(), addToHistory()