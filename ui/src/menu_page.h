#pragma once

#include <QWidget>
#include <QListWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include "FileManager.h"

// 菜单页
// 包含：左侧分类列表、右侧推荐栏 + 菜品卡片滚动区
class MenuPage : public QWidget
{
    Q_OBJECT

public:
    explicit MenuPage(QWidget *parent = nullptr);

    // 加载数据（只需调用一次，或换用户后更新折扣率时不需要重新加载）
    void setData(const QList<Dish_qt> &allItems,
                 const QList<Dish_qt> &bySales,
                 const QList<Dish_qt> &byRating,
                 const QList<Dish_qt> &byComments,
                 const QStringList    &categories);

    // 更新当前用户折扣率并刷新菜品卡片（登录成功后调用）
    void setDiscountRate(double rate);

    // 当前选中的分类文字
    QString currentCategory() const;

signals:
    void addDishClicked(int dishId);      // 点击 + 加入购物车
    void dishCountChanged(int count);     // 当前展示菜品数量变化

private slots:
    void onCategoryChanged(int row);
    void switchRecommendMethod(int index);

private:
    void setupUI();
    void refreshDishList(const QString &category);

    // 数据
    QList<Dish_qt> m_allItems;
    QList<Dish_qt> m_bySales;
    QList<Dish_qt> m_byRating;
    QList<Dish_qt> m_byComments;
    double  m_discountRate = 1.0;
    QString m_recommendMethod = "销量最高";

    // UI
    QListWidget *m_categoryList;
    QFrame *m_recommendBar;
    QPushButton *m_mostSaled;
    QPushButton *m_highScore;
    QPushButton *m_mostCommented;
    QWidget *m_dishContainer;
    QVBoxLayout *m_dishListLayout;
};
