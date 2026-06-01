#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
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

private:
    void loadData();
    void setupUI();
    void refreshDishList(const QString &category = QString());

    Ui_order_system *ui;

    // 左侧分类列表
    QListWidget  *m_categoryList;

    // 右侧菜品区域
    QScrollArea  *m_scrollArea;
    QWidget      *m_dishContainer;
    QVBoxLayout  *m_dishListLayout;

    // 数据
    QList<Dish_qt> m_allItems;
    QStringList     m_categories;
};
