#include "order_system.h"
#include "checkout_dialog.h"

#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
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

    auto *historyOrderBtn = new QPushButton("历史订单点菜", m_navBar);
    historyOrderBtn->setVisible(false);
    auto showHistoryOrderDialog = [this]() {
        if (m_orderService == nullptr || m_currentUser.name.empty()) {
            QMessageBox::information(this, "提示", "请先登录后再查看历史订单。");
            return;
        }

        auto historyOrders = OrderService::loadUserHistoryOrders(
            m_currentUser, "../storage/data/history_order.txt");

        QList<QList<Dish_qt>> matchedOrders;
        QStringList displayLines;
        for (size_t i = 0; i < historyOrders.size(); i++) {
            QList<Dish_qt> matchedDishes;
            QStringList dishNames;
            int missingCount = 0;

            for (const auto &historyDish : historyOrders[i]) {
                const QString historyName = QString::fromStdString(historyDish.name);
                bool found = false;
                for (const auto &item : m_allItems) {
                    if (item.name == historyName) {
                        matchedDishes.append(item);
                        dishNames.append(item.name);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    missingCount++;
                }
            }

            if (matchedDishes.isEmpty()) {
                continue;
            }

            QString line = QString("历史订单 #%1：%2").arg(matchedOrders.size() + 1).arg(dishNames.join("、"));
            if (missingCount > 0) {
                line += QString("（%1 道菜当前菜单中不存在）").arg(missingCount);
            }
            matchedOrders.append(matchedDishes);
            displayLines.append(line);
        }

        if (matchedOrders.isEmpty()) {
            QMessageBox::information(this, "提示", "当前用户暂无可用的历史订单。");
            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle("历史订单点菜");
        dialog.resize(560, 360);

        auto *dialogLayout = new QVBoxLayout(&dialog);
        dialogLayout->setContentsMargins(16, 16, 16, 16);
        dialogLayout->setSpacing(10);

        auto *tipLabel = new QLabel("选择一条历史订单，确认后会直接加入购物车。", &dialog);
        tipLabel->setStyleSheet("font-size: 13px; color: #666666;");
        dialogLayout->addWidget(tipLabel);

        auto *orderList = new QListWidget(&dialog);
        orderList->setStyleSheet(R"(
            QListWidget {
                background: #FFFFFF; border: 1px solid #E8E8E8; border-radius: 6px;
                font-size: 13px; color: #333333; outline: none;
            }
            QListWidget::item { padding: 10px; border-bottom: 1px solid #F5F5F5; }
            QListWidget::item:selected { background: #EAF4FF; color: #0085FF; }
        )");
        for (int i = 0; i < displayLines.size(); i++) {
            auto *item = new QListWidgetItem(displayLines[i], orderList);
            item->setData(Qt::UserRole, i);
        }
        orderList->setCurrentRow(0);
        dialogLayout->addWidget(orderList, 1);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        buttonBox->button(QDialogButtonBox::Ok)->setText("加入购物车");
        buttonBox->button(QDialogButtonBox::Cancel)->setText("取消");
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        connect(orderList, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);
        dialogLayout->addWidget(buttonBox);

        if (dialog.exec() != QDialog::Accepted || orderList->currentItem() == nullptr) {
            return;
        }

        const int selectedIndex = orderList->currentItem()->data(Qt::UserRole).toInt();
        const QList<Dish_qt> selectedOrder = matchedOrders[selectedIndex];
        for (const auto &item : selectedOrder) {
            Dish d = m_fl.dish_to_cpp(item);
            m_orderService->addDish(d);
        }

        QList<Dish_qt> qtOrder = buildQtOrder();
        m_navBar->setDishCount(qtOrder.size());
        m_cartPage->setCart(qtOrder, m_orderService->getDiscountRate(m_currentUser));
        switchPage(3);
    };

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
    connect(m_cartPage, &CartPage::historyOrderRequested, this, showHistoryOrderDialog);

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

        // 结算前保存订单快照（checkout 后会 clearOrder）
        QList<Dish_qt> orderedDishes = buildQtOrder();

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
        scheduleAutoAdvance();  // 启动随机自动叫号

        // 收集本次订单的菜品 ID 和名称（去重）
        QList<int> dishIds;
        QStringList dishNames;
        for (const auto &d : orderedDishes) {
            if (!dishIds.contains(d.id)) {
                dishIds.append(d.id);
                dishNames.append(d.name);
            }
        }

        // 弹出评论弹窗
        CheckoutDialog dlg(dishNames, dishIds, total, m_myQueueId,
                           m_currentUser.id, this);
        dlg.exec();

        CommentMsg cm = dlg.getComment();
        if (cm.rate > 0) {
            m_fl.AddCommentAndUpdateMenu(cm);
            // 重新加载菜单和评论数据（评分和评论数已更新）
            m_fl.LoadMenu();
            m_fl.LoadComments();
            m_allItems    = m_fl.getMenu_qt();
            m_bySales     = m_fl.getRecommendBySales();
            m_byRating    = m_fl.getRecommendByRating();
            m_byComments  = m_fl.getRecommendByComments();
            m_allComments = m_fl.getComments();
        }

        switchPage(2);
    });

    // ---- 排队页信号连接 ----
    connect(m_queuePage, &QueuePage::refreshRequested, this, [this]() {
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


//  随机自动叫号
void order_system::scheduleAutoAdvance()
{
    if (m_queueService.is_Empty()) {
        return;
    }

    // 生成 5~20 秒随机延迟（毫秒）
    int delaySec  = QRandomGenerator::global()->bounded(5, 21);
    int delayMsec = delaySec * 1000;

    QTimer::singleShot(delayMsec, this, &order_system::onAutoAdvance);
}

void order_system::onAutoAdvance()
{
    if (!m_queueService.is_Empty()) {
        m_queueService.advance_queue();   // 预约队列第一人 → 取餐队列
        refreshQueuePage();
    }

    scheduleAutoAdvance();  // 调度下一次随机叫号
}

