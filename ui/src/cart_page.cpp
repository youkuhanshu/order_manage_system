#include "cart_page.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

// 与 dish_card.cpp 相同的分类色板
static const char *cartCategoryColor(const QString &category)
{
    if (category == "热菜") return "#FF6B4A";
    if (category == "凉菜") return "#58D68D";
    if (category == "汤羹") return "#E8A87C";
    if (category == "主食") return "#FFB347";
    if (category == "饮品") return "#5DADE2";
    if (category == "甜品") return "#FF8CA3";
    if (category == "烧烤") return "#E74C3C";
    if (category == "小吃") return "#F39C12";
    return "#BBBBBB";
}

// ================================================================
//  CartPage
// ================================================================
CartPage::CartPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void CartPage::setCart(const QList<Dish_qt> &items, double discountRate)
{
    m_discountRate = discountRate;
    m_counts.clear();
    m_dishes.clear();

    // 统计数量，保持首次出现顺序
    for (size_t i = 0; i < (size_t)items.size(); i++) {
        if (!m_counts.contains(items[i].id)) {
            m_dishes.append(items[i]);
        }
        m_counts[items[i].id]++;
    }

    refreshDisplay();
}

void CartPage::setupUI()
{
    setStyleSheet("background: #F5F5F5;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 空购物车状态 ----
    m_emptyWidget = new QWidget(this);
    auto *emptyLayout = new QVBoxLayout(m_emptyWidget);
    emptyLayout->setAlignment(Qt::AlignCenter);
    emptyLayout->setSpacing(12);

    auto *emptyIcon = new QLabel(QString::fromUtf8("🛒"), m_emptyWidget);
    emptyIcon->setAlignment(Qt::AlignCenter);
    emptyIcon->setStyleSheet("font-size: 56px; border: none; background: transparent;");
    emptyLayout->addWidget(emptyIcon);

    auto *emptyText = new QLabel("购物车是空的", m_emptyWidget);
    emptyText->setAlignment(Qt::AlignCenter);
    emptyText->setStyleSheet(
        "font-size: 16px; color: #AAAAAA; border: none; background: transparent;");
    emptyLayout->addWidget(emptyText);

    emptyLayout->addSpacing(8);

    auto *goMenuBtn = new QPushButton("去挑选菜品 →", m_emptyWidget);
    goMenuBtn->setFixedSize(150, 42);
    goMenuBtn->setCursor(Qt::PointingHandCursor);
    goMenuBtn->setStyleSheet(R"(
        QPushButton {
            background: #0085FF; color: #FFFFFF; border: none;
            border-radius: 21px; font-size: 14px;
        }
        QPushButton:hover   { background: #0073E6; }
        QPushButton:pressed { background: #0060BF; }
    )");
    connect(goMenuBtn, &QPushButton::clicked, this, &CartPage::backToMenuRequested);
    emptyLayout->addWidget(goMenuBtn, 0, Qt::AlignCenter);

    auto *historyOrderBtn = new QPushButton("历史订单点菜", m_emptyWidget);
    historyOrderBtn->setFixedSize(150, 38);
    historyOrderBtn->setCursor(Qt::PointingHandCursor);
    historyOrderBtn->setStyleSheet(R"(
        QPushButton {
            background: #FFFFFF; color: #0085FF; border: 1px solid #BBDFFF;
            border-radius: 19px; font-size: 14px;
        }
        QPushButton:hover   { background: #EAF4FF; }
        QPushButton:pressed { background: #DCEEFF; }
    )");
    connect(historyOrderBtn, &QPushButton::clicked, this, &CartPage::historyOrderRequested);
    emptyLayout->addWidget(historyOrderBtn, 0, Qt::AlignCenter);

    mainLayout->addWidget(m_emptyWidget, 1);

    // ---- 有内容状态 ----
    m_contentWidget = new QWidget(this);
    auto *contentLayout = new QHBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(12);

    // -- 左侧：已选商品列表 --
    auto *leftCard = new QFrame(m_contentWidget);
    leftCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px; border: 1px solid #F0F0F0; }");
    auto *leftLayout = new QVBoxLayout(leftCard);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    // 左侧顶栏
    auto *listHeader = new QFrame(leftCard);
    listHeader->setFixedHeight(48);
    listHeader->setStyleSheet(
        "QFrame { background: transparent; border: none;"
        "border-bottom: 1px solid #F5F5F5; }");
    auto *listHeaderLayout = new QHBoxLayout(listHeader);
    listHeaderLayout->setContentsMargins(16, 0, 16, 0);

    m_headerCountLabel = new QLabel("已选商品", listHeader);
    m_headerCountLabel->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    listHeaderLayout->addWidget(m_headerCountLabel, 1);

    auto *clearBtn = new QPushButton("清空", listHeader);
    clearBtn->setCursor(Qt::PointingHandCursor);
    clearBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 13px; color: #AAAAAA; }"
        "QPushButton:hover { color: #FF4D2E; }");
    connect(clearBtn, &QPushButton::clicked, this, &CartPage::clearCartRequested);
    listHeaderLayout->addWidget(clearBtn);

    leftLayout->addWidget(listHeader);

    // 菜品滚动列表
    auto *scrollArea = new QScrollArea(leftCard);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical {
            width: 6px; background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #CCCCCC; border-radius: 3px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #AAAAAA; }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0px; }
    )");

    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background: transparent;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(12, 8, 12, 8);
    m_listLayout->setSpacing(8);
    m_listLayout->addStretch();

    scrollArea->setWidget(m_listContainer);
    leftLayout->addWidget(scrollArea, 1);

    contentLayout->addWidget(leftCard, 1);

    // -- 右侧：订单汇总 --
    auto *rightCard = new QFrame(m_contentWidget);
    rightCard->setFixedWidth(280);
    rightCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px; border: 1px solid #F0F0F0; }");
    auto *rightLayout = new QVBoxLayout(rightCard);
    rightLayout->setContentsMargins(16, 0, 16, 16);
    rightLayout->setSpacing(0);

    // 右侧标题
    auto *summaryHeader = new QLabel("订单信息", rightCard);
    summaryHeader->setFixedHeight(48);
    summaryHeader->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #333333;"
        "border-bottom: 1px solid #F5F5F5; border-top: none;"
        "border-left: none; border-right: none; background: transparent;");
    rightLayout->addWidget(summaryHeader);

    rightLayout->addSpacing(8);

    // 汇总条目（可滚动，最多显示 180px）
    auto *summaryScroll = new QScrollArea(rightCard);
    summaryScroll->setWidgetResizable(true);
    summaryScroll->setMaximumHeight(180);
    summaryScroll->setStyleSheet(
        "QScrollArea { background: transparent; border: none; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #DDDDDD; border-radius: 2px; }");

    auto *summaryContainer = new QWidget();
    summaryContainer->setStyleSheet("background: transparent;");
    m_summaryItemsLayout = new QVBoxLayout(summaryContainer);
    m_summaryItemsLayout->setContentsMargins(0, 0, 4, 0);
    m_summaryItemsLayout->setSpacing(6);

    summaryScroll->setWidget(summaryContainer);
    rightLayout->addWidget(summaryScroll);

    rightLayout->addSpacing(10);

    // 分隔线
    auto *divider = new QFrame(rightCard);
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet(
        "QFrame { background: #F0F0F0; border: none; max-height: 1px; }");
    rightLayout->addWidget(divider);

    rightLayout->addSpacing(12);

    // 商品合计
    auto *subtotalRow = new QHBoxLayout();
    auto *subtotalTitle = new QLabel("商品合计", rightCard);
    subtotalTitle->setStyleSheet(
        "font-size: 13px; color: #666666; border: none; background: transparent;");
    m_subtotalLabel = new QLabel("¥0", rightCard);
    m_subtotalLabel->setStyleSheet(
        "font-size: 13px; color: #333333; border: none; background: transparent;");
    subtotalRow->addWidget(subtotalTitle);
    subtotalRow->addStretch();
    subtotalRow->addWidget(m_subtotalLabel);
    rightLayout->addLayout(subtotalRow);

    // 会员折扣行（折扣率 < 1 时显示）
    m_discountRow = new QFrame(rightCard);
    m_discountRow->setStyleSheet("QFrame { border: none; background: transparent; }");
    auto *discountRowLayout = new QHBoxLayout(m_discountRow);
    discountRowLayout->setContentsMargins(0, 6, 0, 0);
    auto *discountTitle = new QLabel("会员折扣", rightCard);
    discountTitle->setStyleSheet(
        "font-size: 13px; color: #E8960A; border: none; background: transparent;");
    m_discountLabel = new QLabel("-¥0", rightCard);
    m_discountLabel->setStyleSheet(
        "font-size: 13px; color: #E8960A; border: none; background: transparent;");
    discountRowLayout->addWidget(discountTitle);
    discountRowLayout->addStretch();
    discountRowLayout->addWidget(m_discountLabel);
    rightLayout->addWidget(m_discountRow);

    rightLayout->addSpacing(12);

    // 实付款
    auto *totalRow = new QHBoxLayout();
    auto *totalTitle = new QLabel("实付款", rightCard);
    totalTitle->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    m_totalLabel = new QLabel("¥0", rightCard);
    m_totalLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #FF4D2E;"
        "border: none; background: transparent;");
    totalRow->addWidget(totalTitle, 0, Qt::AlignVCenter);
    totalRow->addStretch();
    totalRow->addWidget(m_totalLabel, 0, Qt::AlignVCenter);
    rightLayout->addLayout(totalRow);

    rightLayout->addStretch();

    // 去结算按钮
    m_checkoutBtn = new QPushButton("去结算", rightCard);
    m_checkoutBtn->setFixedHeight(44);
    m_checkoutBtn->setCursor(Qt::PointingHandCursor);
    m_checkoutBtn->setStyleSheet(R"(
        QPushButton {
            background: #FF6200; color: #FFFFFF; border: none;
            border-radius: 8px; font-size: 16px; font-weight: bold;
        }
        QPushButton:hover   { background: #E55A00; }
        QPushButton:pressed { background: #CC5000; }
    )");
    connect(m_checkoutBtn, &QPushButton::clicked, this, &CartPage::checkoutRequested);
    rightLayout->addWidget(m_checkoutBtn);

    rightLayout->addSpacing(8);

    // 继续点餐
    auto *backBtn = new QPushButton("← 继续点餐", rightCard);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 13px; color: #AAAAAA; }"
        "QPushButton:hover { color: #0085FF; }");
    connect(backBtn, &QPushButton::clicked, this, &CartPage::backToMenuRequested);
    rightLayout->addWidget(backBtn, 0, Qt::AlignCenter);

    contentLayout->addWidget(rightCard);
    mainLayout->addWidget(m_contentWidget, 1);

    // 初始显示空状态
    m_emptyWidget->setVisible(true);
    m_contentWidget->setVisible(false);
}

void CartPage::refreshDisplay()
{
    bool isEmpty = m_counts.isEmpty();
    m_emptyWidget->setVisible(isEmpty);
    m_contentWidget->setVisible(!isEmpty);

    if (isEmpty) return;

    // ---- 重建菜品列表 ----
    QLayoutItem *child;
    while ((child = m_listLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    int totalCount = 0;
    for (int i = 0; i < m_dishes.size(); i++) {
        int cnt = m_counts.value(m_dishes[i].id, 0);
        if (cnt > 0) {
            m_listLayout->addWidget(makeItemRow(m_dishes[i], cnt));
            totalCount += cnt;
        }
    }
    m_listLayout->addStretch();

    m_headerCountLabel->setText(QString("已选商品（%1件）").arg(totalCount));

    // ---- 重建右侧汇总条目 ----
    QLayoutItem *si;
    while ((si = m_summaryItemsLayout->takeAt(0)) != nullptr) {
        if (si->widget()) si->widget()->deleteLater();
        if (si->layout()) {
            QLayoutItem *li;
            while ((li = si->layout()->takeAt(0)) != nullptr) {
                if (li->widget()) li->widget()->deleteLater();
                delete li;
            }
        }
        delete si;
    }

    double subtotal = 0.0;
    for (int i = 0; i < m_dishes.size(); i++) {
        int cnt = m_counts.value(m_dishes[i].id, 0);
        if (cnt == 0) continue;

        double lineTotal = m_dishes[i].price * cnt;
        subtotal += lineTotal;

        auto *row = new QHBoxLayout();
        auto *nameLabel = new QLabel(
            QString("%1 × %2").arg(m_dishes[i].name).arg(cnt));
        nameLabel->setStyleSheet(
            "font-size: 12px; color: #555555; border: none; background: transparent;");
        auto *priceLabel = new QLabel(QString("¥%1").arg(lineTotal, 0, 'f', 0));
        priceLabel->setStyleSheet(
            "font-size: 12px; color: #333333; border: none; background: transparent;");
        row->addWidget(nameLabel, 1);
        row->addWidget(priceLabel);
        m_summaryItemsLayout->addLayout(row);
    }

    // ---- 更新金额标签 ----
    double discount = subtotal * (1.0 - m_discountRate);
    double total    = subtotal * m_discountRate;

    m_subtotalLabel->setText(QString("¥%1").arg(subtotal, 0, 'f', 0));

    if (m_discountRate < 1.0) {
        m_discountLabel->setText(QString("-¥%1").arg(discount, 0, 'f', 0));
        m_discountRow->setVisible(true);
    } else {
        m_discountRow->setVisible(false);
    }

    m_totalLabel->setText(QString("¥%1").arg(total, 0, 'f', 0));
}

QFrame *CartPage::makeItemRow(const Dish_qt &dish, int count)
{
    auto *row = new QFrame(m_listContainer);
    row->setObjectName("cartItemRow");
    row->setStyleSheet(R"(
        #cartItemRow {
            background: #FFFFFF;
            border-radius: 8px;
            border: 1px solid #F5F5F5;
        }
        #cartItemRow:hover {
            border: 1px solid #E8E8E8;
        }
    )");
    row->setFixedHeight(72);
    row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(12, 10, 12, 10);
    rowLayout->setSpacing(12);

    // 色块头像
    auto *avatar = new QFrame(row);
    avatar->setFixedSize(40, 40);
    avatar->setStyleSheet(QString("QFrame { background: %1; border-radius: 6px; }")
        .arg(cartCategoryColor(dish.category)));
    auto *avatarLayout = new QVBoxLayout(avatar);
    avatarLayout->setContentsMargins(0, 0, 0, 0);
    auto *avatarText = new QLabel(dish.name.at(0), avatar);
    avatarText->setAlignment(Qt::AlignCenter);
    avatarText->setStyleSheet(
        "color: #FFFFFF; font-size: 18px; font-weight: bold;"
        "border: none; background: transparent;");
    avatarLayout->addWidget(avatarText);
    rowLayout->addWidget(avatar);

    // 菜名 + 单价
    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    auto *nameLabel = new QLabel(dish.name, row);
    nameLabel->setStyleSheet(
        "font-size: 14px; font-weight: 600; color: #333333;"
        "border: none; background: transparent;");
    infoLayout->addWidget(nameLabel);

    auto *unitPriceLabel = new QLabel(
        QString("¥%1 / 份").arg(dish.price, 0, 'f', 0), row);
    unitPriceLabel->setStyleSheet(
        "font-size: 11px; color: #AAAAAA; border: none; background: transparent;");
    infoLayout->addWidget(unitPriceLabel);

    rowLayout->addLayout(infoLayout, 1);

    // − 数量 +
    auto *countLayout = new QHBoxLayout();
    countLayout->setSpacing(6);

    auto *minusBtn = new QPushButton(QString::fromUtf8("−"), row);
    minusBtn->setFixedSize(26, 26);
    minusBtn->setCursor(Qt::PointingHandCursor);
    minusBtn->setStyleSheet(R"(
        QPushButton {
            background: #F5F5F5; color: #666666; border: none;
            border-radius: 13px; font-size: 16px; font-weight: bold;
        }
        QPushButton:hover   { background: #E8E8E8; }
        QPushButton:pressed { background: #DDDDDD; }
    )");
    connect(minusBtn, &QPushButton::clicked, this, [this, dish]() {
        emit decreaseRequested(dish.id);
    });
    countLayout->addWidget(minusBtn);

    auto *countLabel = new QLabel(QString::number(count), row);
    countLabel->setFixedWidth(28);
    countLabel->setAlignment(Qt::AlignCenter);
    countLabel->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    countLayout->addWidget(countLabel);

    auto *plusBtn = new QPushButton("+", row);
    plusBtn->setFixedSize(26, 26);
    plusBtn->setCursor(Qt::PointingHandCursor);
    plusBtn->setStyleSheet(R"(
        QPushButton {
            background: #ffd500; color: #FFFFFF; border: none;
            border-radius: 13px; font-size: 18px; font-weight: bold;
        }
        QPushButton:hover   { background: #e6b000; }
        QPushButton:pressed { background: #daa401; }
    )");
    connect(plusBtn, &QPushButton::clicked, this, [this, dish]() {
        emit increaseRequested(dish.id);
    });
    countLayout->addWidget(plusBtn);

    rowLayout->addLayout(countLayout);

    // 小计
    auto *totalLabel = new QLabel(
        QString("¥%1").arg(dish.price * count, 0, 'f', 0), row);
    totalLabel->setFixedWidth(52);
    totalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    totalLabel->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #FF4D2E;"
        "border: none; background: transparent;");
    rowLayout->addWidget(totalLabel);

    return row;
}
