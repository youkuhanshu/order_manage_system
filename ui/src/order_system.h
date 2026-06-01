#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include "ui_order_system.h"
#include "FileManager.h"

class order_system : public QMainWindow
{
    Q_OBJECT

public:
    explicit order_system(QWidget *parent = nullptr);
    ~order_system();

private slots:
    void onCategoryChanged(int row);
    void onAddDish(int dishId);
    void switchPage(int index);

private:
    void loadData();
    void setupUI();
    void refreshDishList(const QString &category = QString());

    Ui_order_system *ui;

    // 导航按钮
    QPushButton  *m_btnMenu;
    QPushButton  *m_btnCart;
    QPushButton  *m_btnQueue;

    // 页面容器
    QStackedWidget *m_stackedWidget;

    // 三个页面
    QWidget *m_menuPage;
    QWidget *m_cartPage;
    QWidget *m_queuePage;

    // 菜单页：左侧分类列表
    QListWidget  *m_categoryList;

    // 菜单页：右侧菜品区域
    QScrollArea  *m_scrollArea;
    QWidget      *m_dishContainer;
    QVBoxLayout  *m_dishListLayout;

    // 数据
    QList<Dish_qt> m_allItems;
    QList<Dish_qt> m_recommendItems;
    QStringList     m_categories;

    // label
    QLabel *m_dishcount;
};
