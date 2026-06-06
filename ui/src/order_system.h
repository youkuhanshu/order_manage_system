#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "ui_order_system.h"
#include "FileManager.h"
#include "Order_mgr.hpp"
#include "nav_bar.h"
#include "auth_page.h"
#include "menu_page.h"
#include "cart_page.h"
#include "queue_page.h"

/// 主窗口
/// 职责：组装各页面组件、处理登录/注册业务逻辑、在各组件之间传递信号
class order_system : public QMainWindow
{
    Q_OBJECT

public:
    explicit order_system(QWidget *parent = nullptr);
    ~order_system();

private:
    void loadData();
    void setupUI();
    void switchPage(int index);
    bool checkUser(const QString &name, const QString &password);
    void doRegister(const QString &name, const QString &password);
    User u;

    // 登录成功后计算折扣率
    double discountRateForUser(const User &u) const;

    // 界面和组件
    Ui_order_system *ui;

    NavBar *m_navBar;
    QStackedWidget *m_stackedWidget;
    LoginPage *m_loginPage;
    RegisterPage *m_registerPage;
    MenuPage *m_menuPage;
    CartPage *m_cartPage;
    QueuePage *m_queuePage;

    // 数据
    QList<Dish_qt> m_allItems;
    QList<Dish_qt> m_bySales;
    QList<Dish_qt> m_byRating;
    QList<Dish_qt> m_byComments;
    std::vector<CommentMsg> m_allComments;
    QStringList m_categories;
    std::vector<User> m_users;
    User m_currentUser;
};
