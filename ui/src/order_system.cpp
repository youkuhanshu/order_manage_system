#include "order_system.h"
#include "dish_card.h"

#include <QApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QMessageBox>

order_system::order_system(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui_order_system)
    , m_btnMenu(nullptr)
    , m_btnCart(nullptr)
    , m_btnQueue(nullptr)
    , m_stackedWidget(nullptr)
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
    m_allItems  = fileManager.getMenu_qt();
    m_categories = fileManager.getCategories_qt();
    m_recommendItems = fileManager.getRecommend_qt();

    if (m_allItems.isEmpty()) {
        QMessageBox::warning(this, "加载失败", QString("未能加载菜单数据\n请确认文件存在且格式正确!"));
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
    auto *topBar = new QFrame(ui->centralwidget);
    topBar->setFixedHeight(54);
    topBar->setStyleSheet("QFrame { background: #FFFFFF; border-bottom: 1px solid #E8E8E8; }");

    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);
    topLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *icon = new QLabel(QString::fromUtf8("🍽"), topBar);
    icon->setStyleSheet("font-size: 24px; border: none; background: transparent;");
    topLayout->addWidget(icon);

    auto *title = new QLabel("饱了么", topBar);
    title->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;"
    );
    topLayout->addWidget(title);

    topLayout->addSpacing(30);

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

    m_btnMenu = new QPushButton("菜品菜单", topBar);
    m_btnMenu->setStyleSheet(navBtnStyle);
    m_btnMenu->setCursor(Qt::PointingHandCursor);
    m_btnMenu->setProperty("active", true);  // 默认选中菜单界面
    m_recommendMethod = "销量最高"; // 默认推荐方式为销量最高
    topLayout->addWidget(m_btnMenu);

    m_btnCart = new QPushButton("购物车", topBar);
    m_btnCart->setStyleSheet(navBtnStyle);
    m_btnCart->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(m_btnCart);

    m_btnQueue = new QPushButton("排队进度", topBar);
    m_btnQueue->setStyleSheet(navBtnStyle);
    m_btnQueue->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(m_btnQueue);

    topLayout->addStretch();

    m_dishcount = new QLabel(QString("共 %1 道菜品").arg(m_allItems.size()), topBar);
    m_dishcount->setStyleSheet(
        "font-size: 13px; color: #999999;"
        "border: none; background: transparent;"
    );
    topLayout->addWidget(m_dishcount);

    mainLayout->addWidget(topBar);

    // 页面容器，存放所有页面
    m_stackedWidget = new QStackedWidget(ui->centralwidget);

    

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
    m_stackedWidget->addWidget(m_menuPage);  // index 0

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

    m_stackedWidget->addWidget(m_cartPage);  // index 1

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

    m_stackedWidget->addWidget(m_queuePage);  // index 2

    mainLayout->addWidget(m_stackedWidget, 1);

    // 导航栏
    connect(m_btnMenu, &QPushButton::clicked, this, [this]() { switchPage(0); });
    connect(m_btnCart, &QPushButton::clicked, this, [this]() { switchPage(1); });
    connect(m_btnQueue, &QPushButton::clicked, this, [this]() { switchPage(2); });

    // 状态栏
    ui->statusbar->showMessage("就绪 — 请选择分类浏览菜品");
    ui->statusbar->setStyleSheet(
        "QStatusBar { background: #FAFAFA; color: #999999; font-size: 12px;"
        "border-top: 1px solid #E8E8E8; }");

    // 默认进入推荐界面
    refreshDishList("推荐");
}

// 页面切换
void order_system::switchPage(int index)
{
    m_stackedWidget->setCurrentIndex(index);

    // 更新按钮高亮
    m_btnMenu->setProperty("active", index == 0);
    m_btnCart->setProperty("active", index == 1);
    m_btnQueue->setProperty("active", index == 2);

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

    if (category == "推荐") {
        // 添加不同排序方式
        for (const auto &item : m_recommendItems) {
            auto *card = new DishCard(item, m_dishContainer);
            connect(card, &DishCard::addClicked, this, &order_system::onAddDish);
            m_dishListLayout->addWidget(card);
            dishcount++;
        }
    }
    else {
        // 筛选
        for (const auto &item : m_allItems) {
            if (item.category == category || category == "全部") {
                auto *card = new DishCard(item, m_dishContainer);
                connect(card, &DishCard::addClicked, this, &order_system::onAddDish);
                m_dishListLayout->addWidget(card);
                dishcount++;
            }
        }
    }

    m_dishcount->setText(QString("共 %1 道菜品").arg(dishcount));

    // 结尾弹簧
    m_dishListLayout->addStretch();
}