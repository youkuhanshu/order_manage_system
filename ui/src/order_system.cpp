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
    FileManager fm;
    fm.LoadMenu();
    fm.LoadUsers();
    fm.LoadComments();

    m_allComments = fm.getComments();
    m_allItems = fm.getMenu_qt();
    m_categories = fm.getCategories_qt();
    m_bySales = fm.getRecommendBySales();
    m_byRating = fm.getRecommendByRating();
    m_byComments = fm.getRecommendByComments();
    m_users = fm.getUsers_cpp();

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
            m_navBar->setUser(m_currentUser);
            m_navBar->setVisible(true);
            m_menuPage->setData(m_allItems, m_bySales, m_byRating, m_byComments, m_categories);
            m_menuPage->setDiscountRate(discountRateForUser(m_currentUser));
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
        m_navBar->setUser(m_currentUser);
        m_navBar->setVisible(true);
        m_menuPage->setData(m_allItems, m_bySales, m_byRating, m_byComments, m_categories);
        m_menuPage->setDiscountRate(1.0); // 新用户是 REGULAR
        switchPage(2);
    });
    connect(m_registerPage, &RegisterPage::toLoginClicked, this, [this]() { switchPage(0); });

    // 菜单页
    m_menuPage = new MenuPage();
    connect(m_menuPage, &MenuPage::addDishClicked, this, [this](int dishId) {
        for (const auto &item : m_allItems) {
            if (item.id == dishId) {
                // 待接入
                return;
            }
        }
    });
    connect(m_menuPage, &MenuPage::dishCountChanged, m_navBar, &NavBar::setDishCount);

    // 购物车页 & 排队页
    m_cartPage  = new CartPage();
    m_queuePage = new QueuePage();

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
    m_stackedWidget->setCurrentIndex(index);
    m_navBar->setActiveNav(index);
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

    FileManager f;
    f.addUser(u.id, u.name, u.password);
}

double order_system::discountRateForUser(const User &u) const
{
    if (u.level == "SILVER") return 0.95;
    if (u.level == "GOLD") return 0.85;
    if (u.level == "PLATINUM") return 0.75;
    return 1.0;
}
