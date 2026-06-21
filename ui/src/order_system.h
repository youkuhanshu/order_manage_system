#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "ui_order_system.h"
#include "FileManager.h"
#include "Order_mgr.hpp"
#include "queue_service.h"
#include "Comment_service.h"
#include "nav_bar.h"
#include "auth_page.h"
#include "menu_page.h"
#include "cart_page.h"
#include "queue_page.h"
#include <QTimer>
#include <QMap>

/// 一笔“已下单、待取餐”的订单信息，取餐时用来弹评价窗
struct PendingReview {
    QStringList dishNames;   // 本单菜品名列表
    QList<int>  dishIds;     // 对应菜品 ID
    double      total  = 0.0; // 实付金额
    int         userId = 0;   // 下单用户 ID
};

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
    void scheduleAutoAdvance(); // 用随机间隔调度下一次自动叫号
    void onAutoAdvance();       // 定时器到期：执行叫号并调度下一次
    void onPickup(int queueId); // 顾客点「取餐」：弹评价窗 + 出队
    void openCommentDialog(int dishId); // 浏览菜品评论：查 dish + 从 CommentService 取评论 + 创建 CommentDialog
    bool m_autoAdvancePending = false; // 防止重复调度
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
    QueueService m_queueService;     

    // 评论排序
    CommentService m_commentService; // 评论排序与推荐
    int m_nextOrderId = 101;         // 简单的订单号自增
    int m_myQueueId   = -1;          // 当前用户最近一次的取餐号

    // 取餐号 → 待评价订单信息（结算时存入，取餐时取出弹评价窗）
    QMap<int, PendingReview> m_pendingReviews;

    FileManager m_fl;

    /// 把 m_orderService->getOrder() 转成 QList<Dish_qt> 供 CartPage 使用
    QList<Dish_qt> buildQtOrder();
};
