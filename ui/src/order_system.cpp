#include "order_system.h"
#include "dish_card.h"

#include <QApplication>
#include <QDir>
#include <QHBoxLayout>
#include <QMessageBox>

// ================================================================
//  构造 / 析构
// ================================================================

order_system::order_system(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui_order_system)
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

    if (m_allItems.isEmpty()) {
        QMessageBox::warning(this, "加载失败", QString("未能加载菜单数据\n请确认文件存在且格式正确!"));
    }
}

// ================================================================
//  主界面布局
// ================================================================

void order_system::setupUI()
{
    // ---- 窗口属性 ----
    setWindowTitle("订单管理系统 — 菜品菜单");
    resize(1000, 680);
    setMinimumSize(800, 600);

    // ---- 全局背景 ----
    setStyleSheet("QMainWindow { background: #F5F5F5; }");
    ui->centralwidget->setStyleSheet("background: #F5F5F5;");

    auto *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ============================================================
    //  顶部标题栏
    // ============================================================
    auto *topBar = new QFrame(ui->centralwidget);
    topBar->setFixedHeight(54);
    topBar->setStyleSheet(
        "QFrame { background: #FFFFFF; border-bottom: 1px solid #E8E8E8; }");

    auto *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 0, 20, 0);
    topLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *icon = new QLabel(QString::fromUtf8("🍽"), topBar);
    icon->setStyleSheet("font-size: 24px; border: none; background: transparent;");
    topLayout->addWidget(icon);

    auto *title = new QLabel("菜品菜单", topBar);
    title->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    topLayout->addWidget(title);

    topLayout->addStretch();

    auto *dishCount = new QLabel(
        QString("共 %1 道菜品").arg(m_allItems.size()), topBar);
    dishCount->setStyleSheet(
        "font-size: 13px; color: #999999;"
        "border: none; background: transparent;");
    topLayout->addWidget(dishCount);

    mainLayout->addWidget(topBar);

    // ============================================================
    //  内容区：左侧分类 + 右侧菜品
    // ============================================================
    auto *contentLayout = new QHBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // ---- 左侧分类列表 ----
    m_categoryList = new QListWidget(ui->centralwidget);
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
            color: #FFC300;
            font-weight: bold;
            background: #FFF9E6;
            border-left: 3px solid #FFC300;
        }
        QListWidget::item:hover {
            color: #FFC300;
            background: #FFFDF5;
        }
    )");

    m_categoryList->addItem("全部");
    for (const auto &cat : m_categories) {
        m_categoryList->addItem(cat);
    }
    m_categoryList->setCurrentRow(0);

    connect(m_categoryList, &QListWidget::currentRowChanged,
            this, &order_system::onCategoryChanged);
    // 发送者，信号，接收者，槽

    contentLayout->addWidget(m_categoryList);

    // ---- 右侧滚动菜品区 ----
    m_scrollArea = new QScrollArea(ui->centralwidget);
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
    contentLayout->addWidget(m_scrollArea, 1);

    mainLayout->addLayout(contentLayout, 1);

    // ---- 状态栏提示 ----
    ui->statusbar->showMessage("就绪 — 请选择分类浏览菜品");
    ui->statusbar->setStyleSheet(
        "QStatusBar { background: #FAFAFA; color: #999999; font-size: 12px;"
        "border-top: 1px solid #E8E8E8; }");

    // ---- 初始加载全部菜品 ----
    refreshDishList();
}

// ================================================================
//  分类切换
// ================================================================

void order_system::onCategoryChanged(int row)
{
    if (row < 0) return;

    const QString cat = (row == 0)
        ? QString()           // "全部" → 空字符串表示不过滤
        : m_categoryList->item(row)->text();

    refreshDishList(cat);
}

// ================================================================
//  加购按钮
// ================================================================

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

// ================================================================
//  刷新菜品列表
// ================================================================

void order_system::refreshDishList(const QString &category)
{
    // 清除旧卡片
    QLayoutItem *child;
    while ((child = m_dishListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    // 筛选
    for (const auto &item : m_allItems) {
        if (category.isEmpty() || item.category == category) {
            auto *card = new DishCard(item, m_dishContainer);
            connect(card, &DishCard::addClicked,
                    this, &order_system::onAddDish);
            m_dishListLayout->addWidget(card);
        }
    }

    // 结尾弹簧
    m_dishListLayout->addStretch();
}