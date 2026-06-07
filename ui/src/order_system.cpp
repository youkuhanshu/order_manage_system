#include "order_system.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QMessageBox>

order_system::order_system(QWidget *parent) : QMainWindow(parent), ui(new Ui_order_system)
{
    ui->setupUi(this);
    loadData();
    setupUI();
}

order_system::~order_system()
{
    delete ui;
}

//  数据加载
void order_system::loadData()
{
    m_fl.LoadMenu();
    m_fl.LoadUsers();
    m_fl.LoadComments();

    m_allComments = m_fl.getComments();
    m_allItems = m_fl.getMenu_qt();
    m_categories = m_fl.getCategories_qt();
    m_bySales = m_fl.getRecommendBySales();
    m_byRating = m_fl.getRecommendByRating();
    m_byComments = m_fl.getRecommendByComments();
    m_users = m_fl.getUsers_cpp();

    if (m_allItems.isEmpty())
        QMessageBox::warning(this, "加载失败", "未能加载菜单数据！");
    if (m_users.empty())
        QMessageBox::warning(this, "加载失败", "未能加载用户数据！");
}

//  界面搭建
void order_system::setupUI()
{
    setWindowTitle("订单管理系统");
    resize(1000, 680);
    setMinimumSize(800, 600);
    setStyleSheet("QMainWindow { background: #F5F5F5; }");
    ui->centralwidget->setStyleSheet("background: #F5F5F5;");

    auto *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 导航栏 ----
    m_navBar = new NavBar(ui->centralwidget);
    m_navBar->setDishCount(m_allItems.size());
    m_navBar->setVisible(false);

    connect(m_navBar, &NavBar::navClicked, this, &order_system::switchPage);
    connect(m_navBar, &NavBar::logoutRequested, this, [this]() {
        m_currentUser = User{};
        m_navBar->setVisible(false);
        switchPage(0);
    });

    mainLayout->addWidget(m_navBar);

    // ---- 页面栈 ----
    m_stackedWidget = new QStackedWidget(ui->centralwidget);

    // 登录页
    m_loginPage = new LoginPage();
    connect(m_loginPage, &LoginPage::loginClicked, this, [this](const QString &name, const QString &password) {
        if (checkUser(name, password)) {
            m_orderService = new OrderService(m_currentUser);
            m_navBar->setUser(m_currentUser);
            m_navBar->setVisible(true);
            m_menuPage->setData(m_allItems, m_bySales, m_byRating, m_byComments, m_categories);
            m_menuPage->setDiscountRate(m_orderService->getDiscountRate(m_currentUser));
            switchPage(2);
        }
        else {
            QMessageBox::warning(this, "登录失败", "用户名或密码不正确！");
        }
    });

    connect(m_loginPage, &LoginPage::toRegisterClicked, this, [this]() { switchPage(1); });

    // 注册页
    m_registerPage = new RegisterPage();
    connect(m_registerPage, &RegisterPage::registerClicked, this, [this](const QString &name, const QString &password) {
        doRegister(name, password);
        m_orderService = new OrderService(m_currentUser);
        m_navBar->setUser(m_currentUser);
        m_navBar->setVisible(true);
        m_menuPage->setData(m_allItems, m_bySales, m_byRating, m_byComments, m_categories);
        m_menuPage->setDiscountRate(m_orderService->getDiscountRate(m_currentUser));
        switchPage(2);
    });

    connect(m_registerPage, &RegisterPage::toLoginClicked, this, [this]() { switchPage(0); });

    // 菜单页
    m_menuPage = new MenuPage();
    connect(m_menuPage, &MenuPage::addDishClicked, this, [this](int dishId) {
        for (const auto &item : m_allItems) {
            if (item.id == dishId) {
                Dish d = m_fl.dish_to_cpp(item);
                m_orderService->addDish(d);
                return;
            }
        }
    });
    connect(m_menuPage, &MenuPage::dishCountChanged, m_navBar, &NavBar::setDishCount);

    // 购物车页 & 排队页
    m_cartPage  = new CartPage();
    m_queuePage = new QueuePage();

    // ---- 购物车信号连接 ----
    connect(m_cartPage, &CartPage::backToMenuRequested, this, [this]() {
        switchPage(2);
    });

    connect(m_cartPage, &CartPage::clearCartRequested, this, [this]() {
        m_orderService->clearOrder();
        m_navBar->setDishCount(0);
        m_cartPage->setCart({}, m_orderService->getDiscountRate(m_currentUser));
    });

    connect(m_cartPage, &CartPage::increaseRequested, this, [this](int dishId) {
        for (const auto &item : m_allItems) {
            if (item.id == dishId) {
                Dish d = m_fl.dish_to_cpp(item);
                m_orderService->addDish(d);
                break;
            }
        }
        QList<Dish_qt> qtOrder = buildQtOrder();
        m_cartPage->setCart(qtOrder, m_orderService->getDiscountRate(m_currentUser));
        m_navBar->setDishCount(qtOrder.size());
    });

    connect(m_cartPage, &CartPage::decreaseRequested, this, [this](int dishId) {
        m_orderService->removeOneDish(dishId);
        QList<Dish_qt> qtOrder = buildQtOrder();
        m_cartPage->setCart(qtOrder, m_orderService->getDiscountRate(m_currentUser));
        m_navBar->setDishCount(qtOrder.size());
    });

    connect(m_cartPage, &CartPage::checkoutRequested, this, [this]() {
        if (m_orderService->getOrder().empty()) {
            QMessageBox::information(this, "提示", "购物车是空的，请先选择菜品。");
            return;
        }
        double total = m_orderService->checkout();

        // 从文件重新加载用户数据，同步最新的等级和消费
        m_fl.LoadUsers();
        m_users = m_fl.getUsers_cpp();
        for (const auto &u : m_users) {
            if (u.name == m_currentUser.name) {
                m_currentUser = u;
                break;
            }
        }

        m_orderService->clearOrder();
        m_navBar->setUser(m_currentUser);
        m_navBar->setDishCount(0);
        m_menuPage->setDiscountRate(m_orderService->getDiscountRate(m_currentUser));

        // 结算后加入排队队列，拿到取餐号
        m_myQueueId = m_queueService.in_queue(m_nextOrderId++);

        QMessageBox::information(this, "结算成功",
            QString("本次实付 ¥%1\n您的取餐号：%2\n可在「排队进度」查看进度")
                .arg(total, 0, 'f', 2).arg(m_myQueueId));
        switchPage(2);
    });

    // ---- 排队页信号连接 ----
    connect(m_queuePage, &QueuePage::refreshRequested, this, [this]() {
        refreshQueuePage();
    });
    connect(m_queuePage, &QueuePage::advanceRequested, this, [this]() {
        m_queueService.advance_queue();   // 前台叫下一位取餐
        refreshQueuePage();
    });
    connect(m_queuePage, &QueuePage::backToMenuRequested, this, [this]() {
        switchPage(2);
    });

    m_stackedWidget->addWidget(m_loginPage);    // index 0 : 登录页
    m_stackedWidget->addWidget(m_registerPage); // index 1 : 注册页
    m_stackedWidget->addWidget(m_menuPage);     // index 2 : 菜单页
    m_stackedWidget->addWidget(m_cartPage);     // index 3 : 购物车
    m_stackedWidget->addWidget(m_queuePage);    // index 4 : 排队

    mainLayout->addWidget(m_stackedWidget, 1);

    // ---- 状态栏 ----
    ui->statusbar->setStyleSheet(
        "QStatusBar { background: #FAFAFA; color: #999999; font-size: 12px;"
        "border-top: 1px solid #E8E8E8; }");
    ui->statusbar->showMessage("就绪 — 请登录");
}

//  页面切换
void order_system::switchPage(int index)
{
    // 切换到购物车页前先刷新显示
    if (index == 3 && m_orderService != nullptr) {
        QList<Dish_qt> qtOrder = buildQtOrder();
        m_cartPage->setCart(qtOrder, m_orderService->getDiscountRate(m_currentUser));
    }
    // 切换到排队页前先刷新进度
    if (index == 4) {
        refreshQueuePage();
    }
    m_stackedWidget->setCurrentIndex(index);
    m_navBar->setActiveNav(index);
}

//  把排队快照刷到 QueuePage
void order_system::refreshQueuePage()
{
    m_queuePage->setQueueData(m_queueService.getCurentCall(),
                              m_queueService.getWaiting(),
                              m_queueService.getTaking(),
                              m_myQueueId);
}

//  把后端 order_ 转成 QList<Dish_qt>
QList<Dish_qt> order_system::buildQtOrder()
{
    QList<Dish_qt> result;
    const std::vector<Dish> &order = m_orderService->getOrder();
    for (size_t i = 0; i < order.size(); i++) {
        Dish d = order[i];
        result.append(m_fl.dish_to_qt(d));
    }
    return result;
}

//  业务逻辑
bool order_system::checkUser(const QString &name, const QString &password)
{
    for (const auto &u : m_users) {
        if (name == QString::fromStdString(u.name)) {
            if (password == QString::fromStdString(u.password)) {
                m_currentUser = u;
                return true;
            }
            return false;
        }
    }
    return false;
}

void order_system::doRegister(const QString &name, const QString &password)
{
    User u;
    u.id = static_cast<int>(m_users.size()) + 1;
    u.name = name.toStdString();
    u.password = password.toStdString();
    u.level = "REGULAR";
    u.total_spent = 0.0;
    m_users.push_back(u);
    m_currentUser = u;

    m_fl.addUser(u.id, u.name, u.password);
}

