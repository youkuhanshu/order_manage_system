#include "order_system.h"
#include "dish_card.h"
#include "comment_dialog.h"

#include <QApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>

order_system::order_system(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui_order_system)
    , m_btnMenu(nullptr)
    , m_btnCart(nullptr)
    , m_btnQueue(nullptr)
    , m_stackedWidget(nullptr)
    , m_loginPage(nullptr)
    , m_registerPage(nullptr)
    , m_menuPage(nullptr)
    , m_cartPage(nullptr)
    , m_queuePage(nullptr)
    , m_categoryList(nullptr)
    , m_scrollArea(nullptr)
    , m_dishContainer(nullptr)
    , m_dishListLayout(nullptr)
{
    ui->setupUi(this);
    loadData();
    setupUI();
}

order_system::~order_system()
{
    delete ui;
}

void order_system::loadData()
{
    FileManager fileManager;
    fileManager.LoadMenu();
    fileManager.LoadUsers();
    fileManager.LoadComments();
    m_allComments = fileManager.getComments();
    m_allItems  = fileManager.getMenu_qt();
    m_categories = fileManager.getCategories_qt();
    m_recommend_by_sales = fileManager.getRecommendBySales();
    m_recommend_by_rating = fileManager.getRecommendByRating();
    m_recommend_by_comments = fileManager.getRecommendByComments();
    m_users = fileManager.getUsers_cpp();

    if (m_allItems.isEmpty()) {
        QMessageBox::warning(this, "加载失败", QString("未能加载菜单数据\n请确认文件存在且格式正确!"));
    }
    if (m_users.size() == 0) {
        QMessageBox::warning(this, "加载失败", QString("未能加载用户数据\n请确认文件存在且格式正确!"));
    }
}

void order_system::setupUI()
{

    // 窗口属性
    setWindowTitle("订单管理系统");
    resize(1000, 680);
    setMinimumSize(800, 600);

    // 背景
    setStyleSheet("QMainWindow { background: #F5F5F5; }");
    ui->centralwidget->setStyleSheet("background: #F5F5F5;");

    auto *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 导航栏
    m_topBar = new QFrame(ui->centralwidget);
    m_topBar->setFixedHeight(54);
    m_topBar->setStyleSheet("QFrame { background: #FFFFFF; border-bottom: 1px solid #E8E8E8; }");

    auto *topLayout = new QHBoxLayout(m_topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);
    topLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // ---- 用户信息区域（可点击，弹出退出登录菜单）----
    m_userPanel = new QFrame(m_topBar);
    m_userPanel->setObjectName("userPanel");
    m_userPanel->setCursor(Qt::PointingHandCursor);
    m_userPanel->setStyleSheet(R"(
        #userPanel {
            background: transparent;
            border: none;
            border-radius: 8px;
        }
        #userPanel:hover {
            background: #F5F5F5;
        }
    )");
    m_userPanel->installEventFilter(this);

    auto *userPanelLayout = new QHBoxLayout(m_userPanel);
    userPanelLayout->setContentsMargins(8, 0, 8, 0);
    userPanelLayout->setSpacing(8);

    m_userAvatarLabel = new QLabel(m_userPanel);
    m_userAvatarLabel->setFixedSize(34, 34);
    m_userAvatarLabel->setAlignment(Qt::AlignCenter);
    m_userAvatarLabel->setStyleSheet(
        "background: #CCCCCC; border-radius: 17px;"
        "color: #FFFFFF; font-size: 14px; font-weight: bold; border: none;");

    auto *userTextLayout = new QVBoxLayout();
    userTextLayout->setSpacing(1);
    userTextLayout->setAlignment(Qt::AlignVCenter);

    m_userNameLabel = new QLabel(m_userPanel);
    m_userNameLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #474747;"
        "border: none; background: transparent;");

    m_userLevelLabel = new QLabel(m_userPanel);
    m_userLevelLabel->setStyleSheet(
        "font-size: 11px; color: #AAAAAA;"
        "border: none; background: transparent;");

    userTextLayout->addWidget(m_userNameLabel);
    userTextLayout->addWidget(m_userLevelLabel);
    userPanelLayout->addWidget(m_userAvatarLabel);
    userPanelLayout->addLayout(userTextLayout);

    topLayout->addWidget(m_userPanel);
    topLayout->addSpacing(20);

    // 导航按钮样式
    QString navBtnStyle = R"(
        QPushButton {
            background: transparent;
            border: none;
            font-size: 14px;
            color: #666666;
            padding: 6px 18px;
        }
        QPushButton:hover { color: #0085FF; }
        QPushButton[active="true"] {
            color: #0085FF;
            font-weight: bold;
            border-bottom: 2px solid #0085FF;
        }
    )";

    m_btnMenu = new QPushButton("菜品菜单", m_topBar);
    m_btnMenu->setStyleSheet(navBtnStyle);
    m_btnMenu->setCursor(Qt::PointingHandCursor);
    // 登录成功后才激活菜单按钮
    m_recommendMethod = "销量最高"; // 默认推荐方式为销量最高
    topLayout->addWidget(m_btnMenu);

    m_btnCart = new QPushButton("购物车", m_topBar);
    m_btnCart->setStyleSheet(navBtnStyle);
    m_btnCart->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(m_btnCart);

    m_btnQueue = new QPushButton("排队进度", m_topBar);
    m_btnQueue->setStyleSheet(navBtnStyle);
    m_btnQueue->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(m_btnQueue);

    topLayout->addStretch();

    m_dishcount = new QLabel(QString("共 %1 道菜品").arg(m_allItems.size()), m_topBar);
    m_dishcount->setStyleSheet(
        "font-size: 13px; color: #999999;"
        "border: none; background: transparent;"
    );
    topLayout->addWidget(m_dishcount);

    mainLayout->addWidget(m_topBar);


    // 页面容器，存放所有页面
    m_stackedWidget = new QStackedWidget(ui->centralwidget);

    // 登录页面
    m_loginPage = new QWidget();
    auto *loginPageLayout = new QVBoxLayout(m_loginPage);
    loginPageLayout->setAlignment(Qt::AlignCenter);
    loginPageLayout->setContentsMargins(0, 0, 0, 0);

    auto *loginCard = new QFrame(m_loginPage); // 登录卡片
    loginCard->setFixedSize(400, 420);
    loginCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px;"
        "border: 1px solid #E8E8E8; }");

    auto *cardLayout = new QVBoxLayout(loginCard);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setContentsMargins(50, 40, 50, 40);
    cardLayout->setSpacing(16);

    // auto *loginIcon = new QLabel(QString::fromUtf8("🍽"), loginCard); // 图标
    // loginIcon->setAlignment(Qt::AlignCenter);
    // loginIcon->setStyleSheet(
    //     "font-size: 40px; border: none; background: transparent;");
    // cardLayout->addWidget(loginIcon);

    auto *nameTitle = new QLabel("饱了么", loginCard); // 标题
    nameTitle->setAlignment(Qt::AlignCenter);
    nameTitle->setStyleSheet(
        "font-size: 30px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    cardLayout->addWidget(nameTitle);

    cardLayout->addSpacing(10);

    auto *loginTitle = new QLabel("登录", loginCard); // 标题
    loginTitle->setAlignment(Qt::AlignCenter);
    loginTitle->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    cardLayout->addWidget(loginTitle);

    cardLayout->addSpacing(20);

    auto *usernameEdit = new QLineEdit(loginCard); // 用户名输入框
    usernameEdit->setPlaceholderText("请输入用户名");
    usernameEdit->setFixedHeight(40);
    usernameEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #E0E0E0;
            border-radius: 6px;
            padding: 0 12px;
            font-size: 14px;
            color: #333333;
            background: #FAFAFA;
        }
        QLineEdit:focus {
            border-color: #0085FF;
            background: #FFFFFF;
        }
    )");
    cardLayout->addWidget(usernameEdit);

    auto *passwordEdit = new QLineEdit(loginCard); // 密码输入框
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setFixedHeight(40);
    passwordEdit->setStyleSheet(usernameEdit->styleSheet());
    cardLayout->addWidget(passwordEdit);

    cardLayout->addSpacing(4);

    auto *loginBtn = new QPushButton("登  录", loginCard); // 登录按钮
    loginBtn->setFixedHeight(42);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setStyleSheet(R"(
        QPushButton {
            background: #0085FF;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover  { background: #0073E6; }
        QPushButton:pressed { background: #0060BF; }
    )");
    cardLayout->addWidget(loginBtn);

    auto *toRegisterBtn = new QPushButton("没有账号？立即注册", loginCard); // 注册入口
    toRegisterBtn->setCursor(Qt::PointingHandCursor);
    toRegisterBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 12px; color: #999999; }"
        "QPushButton:hover { color: #0085FF; }");
    cardLayout->addWidget(toRegisterBtn);

    // 点击登录 → 进入菜单页
    connect(loginBtn, &QPushButton::clicked, this, [this, usernameEdit, passwordEdit]() {
        bool success = checkUser(usernameEdit->text(), passwordEdit->text());
        if (success) {
            updateUserInfo();
            m_topBar->setVisible(true);
            switchPage(2);
            refreshDishList(m_categoryList->currentItem()->text());
        }
        else {
            QMessageBox::warning(this, "登录失败", "用户名或密码不正确！");
        }
    });
    // 点击注册 → 进入注册页
    connect(toRegisterBtn, &QPushButton::clicked, this, [this]() {
        switchPage(1);
    });

    loginPageLayout->addWidget(loginCard);
    m_stackedWidget->addWidget(m_loginPage);  // index 0

    // 注册页面
    m_registerPage = new QWidget();
    auto *regPageLayout = new QVBoxLayout(m_registerPage);
    regPageLayout->setAlignment(Qt::AlignCenter);
    regPageLayout->setContentsMargins(0, 0, 0, 0);

    auto *regCard = new QFrame(m_registerPage);
    regCard->setFixedSize(400, 480);
    regCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px;"
        "border: 1px solid #E8E8E8; }");

    auto *regCardLayout = new QVBoxLayout(regCard);
    regCardLayout->setAlignment(Qt::AlignCenter);
    regCardLayout->setContentsMargins(50, 36, 50, 36);
    regCardLayout->setSpacing(14);

    auto *regIcon = new QLabel(QString::fromUtf8("🍽"), regCard);
    regIcon->setAlignment(Qt::AlignCenter);
    regIcon->setStyleSheet("font-size: 36px; border: none; background: transparent;");
    regCardLayout->addWidget(regIcon);

    auto *regTitle = new QLabel("注册", regCard);
    regTitle->setAlignment(Qt::AlignCenter);
    regTitle->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    regCardLayout->addWidget(regTitle);

    regCardLayout->addSpacing(6);

    auto *regUserEdit = new QLineEdit(regCard);
    regUserEdit->setPlaceholderText("请输入用户名");
    regUserEdit->setFixedHeight(40);
    regUserEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #E0E0E0; border-radius: 6px;
            padding: 0 12px; font-size: 14px; color: #333333;
            background: #FAFAFA;
        }
        QLineEdit:focus { border-color: #0085FF; background: #FFFFFF; }
    )");
    regCardLayout->addWidget(regUserEdit);

    auto *regPwdEdit = new QLineEdit(regCard);
    regPwdEdit->setPlaceholderText("请设置密码");
    regPwdEdit->setEchoMode(QLineEdit::Password);
    regPwdEdit->setFixedHeight(40);
    regPwdEdit->setStyleSheet(regUserEdit->styleSheet());
    regCardLayout->addWidget(regPwdEdit);

    auto *regPwdConfirm = new QLineEdit(regCard);
    regPwdConfirm->setPlaceholderText("请再次输入密码");
    regPwdConfirm->setEchoMode(QLineEdit::Password);
    regPwdConfirm->setFixedHeight(40);
    regPwdConfirm->setStyleSheet(regUserEdit->styleSheet());
    regCardLayout->addWidget(regPwdConfirm);

    regCardLayout->addSpacing(2);

    auto *regBtn = new QPushButton("注  册", regCard);
    regBtn->setFixedHeight(42);
    regBtn->setCursor(Qt::PointingHandCursor);
    regBtn->setStyleSheet(R"(
        QPushButton {
            background: #0085FF; color: #FFFFFF; border: none;
            border-radius: 6px; font-size: 16px; font-weight: bold;
        }
        QPushButton:hover  { background: #0073E6; }
        QPushButton:pressed { background: #0060BF; }
    )");
    regCardLayout->addWidget(regBtn);

    auto *toLoginBtn = new QPushButton("已有账号？立即登录", regCard);
    toLoginBtn->setCursor(Qt::PointingHandCursor);
    toLoginBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 12px; color: #999999; }"
        "QPushButton:hover { color: #0085FF; }");
    regCardLayout->addWidget(toLoginBtn);

    connect(regBtn, &QPushButton::clicked, this, [this, regUserEdit, regPwdEdit, regPwdConfirm]() {
        if (regPwdEdit->text() != regPwdConfirm->text()) {
            QMessageBox::warning(this, "注册失败", "两次输入的密码不相同！");
        }
        else {
            addUser(regUserEdit->text(), regPwdEdit->text());
            updateUserInfo();
            m_topBar->setVisible(true);
            switchPage(2);
            refreshDishList(m_categoryList->currentItem()->text());
        }
    });
    connect(toLoginBtn, &QPushButton::clicked, this, [this]() {
        switchPage(0);
    });

    regPageLayout->addWidget(regCard);
    m_stackedWidget->addWidget(m_registerPage);  // index 1


    // 菜单界面
    m_menuPage = new QWidget();
    auto *menuPageLayout = new QHBoxLayout(m_menuPage);
    menuPageLayout->setContentsMargins(0, 0, 0, 0);
    menuPageLayout->setSpacing(0);

    m_categoryList = new QListWidget(m_menuPage); // 左侧分类列表
    m_categoryList->setFixedWidth(150);
    m_categoryList->setStyleSheet(R"(
        QListWidget {
            background: #FFFFFF;
            border: none;
            border-right: 1px solid #E8E8E8;
            font-size: 14px;
            outline: none;
        }
        QListWidget::item {
            padding: 14px 22px;
            color: #666666;
            border: none;
        }
        QListWidget::item:selected {
            color: #0085FF;
            font-weight: bold;
            background: #EBF5FF;
            border-left: 3px solid #0085FF;
        }
        QListWidget::item:hover {
            color: #0085FF;
            background: #F5FAFF;
        }
    )");

    m_categoryList->addItem("全部");
    m_categoryList->addItem("推荐");
    for (const auto &cat : m_categories) {
        m_categoryList->addItem(cat);
    }
    m_categoryList->setCurrentRow(1);

    connect(m_categoryList, &QListWidget::currentRowChanged,
            this, &order_system::onCategoryChanged);

    menuPageLayout->addWidget(m_categoryList);

    auto *rightContainer = new QWidget(m_menuPage);        // 右侧容器
    auto *rightMenuLayout = new QVBoxLayout(rightContainer);
    rightMenuLayout->setContentsMargins(0, 0, 0, 0);
    rightMenuLayout->setSpacing(0);

    m_recommendBar = new QFrame(rightContainer);
    m_recommendBar->setFixedHeight(40);
    auto *barLayout = new QHBoxLayout(m_recommendBar);
    barLayout->setContentsMargins(16, 0, 16, 0);
    barLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *recommendMethod = new QLabel("推荐方式", m_recommendBar);
    recommendMethod->setStyleSheet(
        "font-size: 12px; font-weight: normal; color: #3d3d3d;"
        "border: none; background: transparent;");
    barLayout->addWidget(recommendMethod);

    QString recBtnStyle = R"(
        QPushButton {
            background: transparent;
            border: 1px solid #E0E0E0;
            border-radius: 4px;
            font-size: 12px;
            color: #666666;
            padding: 4px 14px;
        }
        QPushButton:hover { border-color: #0085FF; color: #0085FF; }
        QPushButton[active="true"] {
            background: #339bfc;
            color: #FFFFFF;
            border-color: #168dfd;
        }
    )";

    m_mostSaled = new QPushButton("销量最高", m_recommendBar);
    m_mostSaled->setStyleSheet(recBtnStyle);
    m_mostSaled->setCursor(Qt::PointingHandCursor);
    m_mostSaled->setProperty("active", true);
    barLayout->addWidget(m_mostSaled);

    m_highScore = new QPushButton("评分最高", m_recommendBar);
    m_highScore->setStyleSheet(recBtnStyle);
    m_highScore->setCursor(Qt::PointingHandCursor);
    barLayout->addWidget(m_highScore);

    m_mostCommented = new QPushButton("最多评价", m_recommendBar);
    m_mostCommented->setStyleSheet(recBtnStyle);
    m_mostCommented->setCursor(Qt::PointingHandCursor);
    barLayout->addWidget(m_mostCommented);

    barLayout->addStretch();

    connect(m_mostSaled, &QPushButton::clicked, this, [this]() { switchRecommendMethod(0); });
    connect(m_highScore, &QPushButton::clicked, this, [this]() { switchRecommendMethod(1); });
    connect(m_mostCommented, &QPushButton::clicked, this, [this]() { switchRecommendMethod(2); });

    rightMenuLayout->addWidget(m_recommendBar);

    m_scrollArea = new QScrollArea(rightContainer); // 右侧滚动菜品区
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            background: #F5F5F5;
            border: none;
        }
        QScrollBar:vertical {
            width: 6px;
            background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #CCCCCC;
            border-radius: 3px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #AAAAAA;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");

    m_dishContainer = new QWidget();
    m_dishContainer->setStyleSheet("background: #F5F5F5;");
    m_dishListLayout = new QVBoxLayout(m_dishContainer);
    m_dishListLayout->setContentsMargins(16, 12, 16, 12);
    m_dishListLayout->setSpacing(10);
    m_dishListLayout->addStretch();

    m_scrollArea->setWidget(m_dishContainer);

    rightMenuLayout->addWidget(m_scrollArea, 1);
    menuPageLayout->addWidget(rightContainer, 1);
    m_stackedWidget->addWidget(m_menuPage);  // index 2


    // 购物车页面
    m_cartPage = new QWidget();
    auto *cartLayout = new QVBoxLayout(m_cartPage);
    cartLayout->setAlignment(Qt::AlignCenter);

    auto *cartPlaceholder = new QLabel("购物车 — 没写", m_cartPage);
    cartPlaceholder->setAlignment(Qt::AlignCenter);
    cartPlaceholder->setStyleSheet(
        "font-size: 20px; color: #CCCCCC; border: none; background: transparent;"
    );
    cartLayout->addWidget(cartPlaceholder);

    m_stackedWidget->addWidget(m_cartPage);  // index 3


    // 排队界面
    m_queuePage = new QWidget();
    auto *queueLayout = new QVBoxLayout(m_queuePage);
    queueLayout->setAlignment(Qt::AlignCenter);

    auto *queuePlaceholder = new QLabel("排队进度 — 没写", m_queuePage);
    queuePlaceholder->setAlignment(Qt::AlignCenter);
    queuePlaceholder->setStyleSheet(
        "font-size: 20px; color: #CCCCCC; border: none; background: transparent;"
    );
    queueLayout->addWidget(queuePlaceholder);

    m_stackedWidget->addWidget(m_queuePage);  // index 4


    mainLayout->addWidget(m_stackedWidget, 1);

    // 导航栏
    connect(m_btnMenu, &QPushButton::clicked, this, [this]() { switchPage(2); });
    connect(m_btnCart, &QPushButton::clicked, this, [this]() { switchPage(3); });
    connect(m_btnQueue, &QPushButton::clicked, this, [this]() { switchPage(4); });

    // 状态栏
    ui->statusbar->showMessage("就绪 — 请选择分类浏览菜品");
    ui->statusbar->setStyleSheet(
        "QStatusBar { background: #FAFAFA; color: #999999; font-size: 12px;"
        "border-top: 1px solid #E8E8E8; }");

    // 默认进入推荐菜单
    refreshDishList("推荐");

    // 开始时导航栏隐藏
    m_topBar->setVisible(false);
}

// 页面切换
void order_system::switchPage(int index)
{
    m_stackedWidget->setCurrentIndex(index);

    // 更新按钮高亮
    m_btnMenu->setProperty("active", index == 2);
    m_btnCart->setProperty("active", index == 3);
    m_btnQueue->setProperty("active", index == 4);

    // 刷新样式（property 改了之后必须重新 polish）
    for (auto *btn : {m_btnMenu, m_btnCart, m_btnQueue}) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
}

//  分类切换
void order_system::onCategoryChanged(int row)
{
    if (row < 0) {
        return;
    }
    else if (row == 1) {
        m_recommendBar->setVisible(true);
    }
    else {
        m_recommendBar->setVisible(false);
    }

    const QString cat = m_categoryList->item(row)->text();
    refreshDishList(cat);
}

// 推荐方式切换
void order_system::switchRecommendMethod(int index) {
    // 更新按钮高亮及推荐方式
    switch (index) {
        case 0:
            m_recommendMethod = "销量最高";
            break;
        case 1:
            m_recommendMethod = "评分最高";
            break;
        case 2:
            m_recommendMethod = "最多评价";
            break;
        default:
            break;
    }
    m_mostSaled->setProperty("active", index == 0);
    m_highScore->setProperty("active", index == 1);
    m_mostCommented->setProperty("active", index == 2);

    // 刷新样式（property 改了之后必须重新 polish）
    for (auto *btn : {m_mostSaled, m_highScore, m_mostCommented}) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }
    
    refreshDishList("推荐");
}

//  加购按钮
void order_system::onAddDish(int dishId)
{
    // 在状态栏给一个反馈（后续可扩展为购物车功能）
    for (const auto &item : m_allItems) {
        if (item.id == dishId) {
            ui->statusbar->showMessage(
                QString("已将「%1」加入购物车").arg(item.name), 2000);
            return;
        }
    }
}

//  刷新菜品列表
void order_system::refreshDishList(const QString &category)
{
    int dishcount = 0;

    // 清除旧卡片
    QLayoutItem *child;
    while ((child = m_dishListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    // 根据当前用户等级计算折扣率
    double discountRate = 1.0;
    if      (m_current_user.level == "SILVER")   discountRate = 0.95;
    else if (m_current_user.level == "GOLD")     discountRate = 0.85;
    else if (m_current_user.level == "PLATINUM") discountRate = 0.75;

    if (category == "推荐") {
        QList<Dish_qt> recommenditems;
        if (m_recommendMethod == "销量最高") {
            recommenditems = m_recommend_by_sales;
        }
        else if (m_recommendMethod == "评分最高") {
            recommenditems = m_recommend_by_rating;
        }
        else if (m_recommendMethod == "最多评价") {
            recommenditems = m_recommend_by_comments;
        }

        for (const auto &item : recommenditems) {
            auto *card = new DishCard(item, discountRate, m_dishContainer);
            connect(card, &DishCard::addClicked, this, &order_system::onAddDish);
            connect(card, &DishCard::commentClicked, this, &order_system::onShowComments);
            m_dishListLayout->addWidget(card);
            dishcount++;
        }
    }
    else {
        // 筛选
        for (const auto &item : m_allItems) {
            if (item.category == category || category == "全部") {
                auto *card = new DishCard(item, discountRate, m_dishContainer);
                connect(card, &DishCard::addClicked, this, &order_system::onAddDish);
                connect(card, &DishCard::commentClicked, this, &order_system::onShowComments);
                m_dishListLayout->addWidget(card);
                dishcount++;
            }
        }
    }

    m_dishcount->setText(QString("共 %1 道菜品").arg(dishcount));

    // 结尾弹簧
    m_dishListLayout->addStretch();
}

// 登录
bool order_system::checkUser(QString name, QString password) {
    for (size_t i = 0;i < m_users.size();i++) {
        if (name == QString::fromStdString(m_users[i].name)) {
            if (password == QString::fromStdString(m_users[i].password)) {
                m_current_user = m_users[i]; // 设置当前用户
                return true; // 登录成功
            }
            else {
                return false; // 登录失败
            }
        }
    }
    return false; // 用户不存在，一样视为登录失败
}

// 添加用户
void order_system::addUser(QString name, QString password) {
    int id = m_users.size() + 1;

    User u;
    u.id = id;
    u.name = name.toStdString();
    u.password = password.toStdString();
    u.level = "REGULAR";
    m_users.push_back(u); // 新用户添加到用户向量

    FileManager f;
    f.addUser(id, u.name, u.password);
}

// 弹出评论弹窗
void order_system::onShowComments(int dishId)
{
    for (const auto &dish : m_allItems) {
        if (dish.id == dishId) {
            CommentDialog dlg(dish, this);
            dlg.exec();
            return;
        }
    }
}

// 刷新顶栏用户信息（登录/注册成功后调用）
void order_system::updateUserInfo()
{
    static const char *avatarColors[] = {
        "#FF6B4A", "#5DADE2", "#58D68D",
        "#AF7AC5", "#F4D03F", "#EB984E"
    };

    const QString name = QString::fromStdString(m_current_user.name);
    int colorIdx = name.isEmpty() ? 0 : (name[0].unicode() % 6);

    m_userAvatarLabel->setText(name.isEmpty() ? "?" : QString(name[0]));
    m_userAvatarLabel->setStyleSheet(QString(
        "background: %1; border-radius: 17px;"
        "color: #FFFFFF; font-size: 14px; font-weight: bold; border: none;")
        .arg(avatarColors[colorIdx]));

    m_userNameLabel->setText(name);

    const std::string &level = m_current_user.level;
    QString levelText;
    if      (level == "PLATINUM") { levelText = "白金会员"; }
    else if (level == "GOLD")     { levelText = "黄金会员"; }
    else if (level == "SILVER")   { levelText = "白银会员"; }
    else                          { levelText = "普通用户"; }

    m_userLevelLabel->setText(levelText);
    m_userLevelLabel->setStyleSheet(QString("font-size: 11px; color: #AAAAAA; border: none; background: transparent;"));
}

// 点击用户信息区域 → 弹出下拉菜单
bool order_system::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_userPanel && event->type() == QEvent::MouseButtonPress) {
        QMenu menu(this);
        menu.setStyleSheet(R"(
            QMenu {
                background: #FFFFFF;
                border: 1px solid #E8E8E8;
                border-radius: 8px;
                padding: 4px 0;
            }
            QMenu::item {
                padding: 10px 28px;
                font-size: 14px;
                color: #333333;
            }
            QMenu::item:selected {
                background: #FFF0EE;
                color: #FF4D2E;
            }
        )");

        QAction *logoutAction = menu.addAction("退出登录");
        connect(logoutAction, &QAction::triggered, this, [this]() {
            m_current_user = User{};
            m_topBar->setVisible(false);
            switchPage(0);
        });

        // 菜单显示在用户面板正下方
        menu.exec(m_userPanel->mapToGlobal(QPoint(0, m_userPanel->height())));
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}
