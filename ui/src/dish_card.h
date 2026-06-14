#pragma once

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include "FileManager.h"

/// 美团风格菜品卡片
class DishCard : public QFrame
{
    Q_OBJECT // 这个宏是固定写法，必须有一个

public:
    /// salesRank / ratingRank：在 Top5 中的名次（1~5），0 表示不在榜
    explicit DishCard(const Dish_qt &dish, double discountRate = 1.0,
                      int salesRank = 0, int ratingRank = 0,
                      QWidget *parent = nullptr);

    int dishId() const { return m_dish.id; }

signals: // 声明一个自定义信号
    void addClicked(int dishId);
    void commentClicked(int dishId);

private:
    void setupUI(const Dish_qt &dish);

    Dish_qt m_dish;
    double m_discountRate;
    int m_salesRank;
    int m_ratingRank;

    QFrame *m_imageFrame; // 左侧图片占位
    QLabel *m_imageText; // 图片占位文字
    QLabel *m_nameLabel; // 菜名
    QLabel *m_descLabel; // 说明
    QLabel *m_ratingLabel; // 评分
    QLabel *m_salesLabel; // 月售
    QLabel *m_priceLabel; // ¥原价
    QLabel *m_memberPriceLabel; // 会员价（nullptr 表示非会员）
    QPushButton *m_addBtn; // 添加按钮
};
