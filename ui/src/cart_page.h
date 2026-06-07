#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QMap>
#include "FileManager.h"

/// 美团风格购物车页
/// 外部接口：setCart() 传入当前订单快照；通过 signals 通知 order_system 处理业务
class CartPage : public QWidget
{
    Q_OBJECT

public:
    explicit CartPage(QWidget *parent = nullptr);

    /// 刷新购物车显示，切换到本页前调用
    /// items：order_ 中所有菜品（含重复，一份一条）
    void setCart(const QList<Dish_qt> &items, double discountRate);

signals:
    void increaseRequested(int dishId);   ///< 点 + 增加一份
    void decreaseRequested(int dishId);   ///< 点 - 减少一份
    void clearCartRequested();            ///< 清空购物车
    void checkoutRequested();             ///< 去结算
    void backToMenuRequested();           ///< 继续点餐

private:
    void setupUI();
    void refreshDisplay();
    QFrame *makeItemRow(const Dish_qt &dish, int count);

    // 数据
    QList<Dish_qt>  m_dishes;           // 去重后的菜品（按首次加入顺序）
    QMap<int, int>  m_counts;           // dishId → 数量
    double          m_discountRate = 1.0;

    // 空购物车 / 有内容 两套 UI（show/hide 切换）
    QWidget     *m_emptyWidget;
    QWidget     *m_contentWidget;

    // 左侧列表区
    QWidget     *m_listContainer;
    QVBoxLayout *m_listLayout;
    QLabel      *m_headerCountLabel;

    // 右侧汇总区
    QVBoxLayout *m_summaryItemsLayout;
    QLabel      *m_subtotalLabel;
    QFrame      *m_discountRow;
    QLabel      *m_discountLabel;
    QLabel      *m_totalLabel;
    QPushButton *m_checkoutBtn;
};
