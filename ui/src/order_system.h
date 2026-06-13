#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "ui_order_system.h"
#include "FileManager.h"
#include "Order_mgr.hpp"
#include "queue_service.h"
#include "nav_bar.h"
#include "auth_page.h"
#include "menu_page.h"
#include "cart_page.h"
#include "queue_page.h"
#include <QTimer>

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
    void refreshQueuePage();   // 把排队快照刷到 QueuePage
    void setupQueueAutoAdvance(); // 配置 QueueService 自动叫号回调
    User u;

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

    // 菜单
    OrderService *m_orderService = nullptr;

    // 排队
    QueueService m_queueService;     // 复用后端排队逻辑
    int m_nextOrderId = 101;         // 简单的订单号自增
    int m_myQueueId   = -1;          // 当前用户最近一次的取餐号

    FileManager m_fl;

    /// 把 m_orderService->getOrder() 转成 QList<Dish_qt> 供 CartPage 使用
    QList<Dish_qt> buildQtOrder();
};
