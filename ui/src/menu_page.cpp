#include "menu_page.h"
#include "dish_card.h"
#include "comment_dialog.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QStyle>

MenuPage::MenuPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void MenuPage::setupUI()
{
    auto *pageLayout = new QHBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // ---- 左侧分类列表 ----
    m_categoryList = new QListWidget(this);
    m_categoryList->setFixedWidth(150);
    m_categoryList->setStyleSheet(R"(
        QListWidget {
            background: #FFFFFF; border: none;
            border-right: 1px solid #E8E8E8;
            font-size: 14px; outline: none;
        }
        QListWidget::item { padding: 14px 22px; color: #666666; border: none; }
        QListWidget::item:selected {
            color: #0085FF; font-weight: bold;
            background: #EBF5FF; border-left: 3px solid #0085FF;
        }
        QListWidget::item:hover { color: #0085FF; background: #F5FAFF; }
    )");

    connect(m_categoryList, &QListWidget::currentRowChanged,
            this, &MenuPage::onCategoryChanged);

    pageLayout->addWidget(m_categoryList);

    // ---- 右侧容器（推荐栏 + 滚动区）----
    auto *rightContainer = new QWidget(this);
    auto *rightLayout    = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 推荐方式切换栏
    m_recommendBar = new QFrame(rightContainer);
    m_recommendBar->setFixedHeight(40);
    auto *barLayout = new QHBoxLayout(m_recommendBar);
    barLayout->setContentsMargins(16, 0, 16, 0);
    barLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto *methodLabel = new QLabel("推荐方式", m_recommendBar);
    methodLabel->setStyleSheet(
        "font-size: 12px; color: #3d3d3d; border: none; background: transparent;");
    barLayout->addWidget(methodLabel);

    const QString recBtnStyle = R"(
        QPushButton {
            background: transparent; border: 1px solid #E0E0E0;
            border-radius: 4px; font-size: 12px; color: #666666; padding: 4px 14px;
        }
        QPushButton:hover { border-color: #0085FF; color: #0085FF; }
        QPushButton[active="true"] {
            background: #339bfc; color: #FFFFFF; border-color: #168dfd;
        }
    )";

    m_mostSaled = new QPushButton("销量最高", m_recommendBar);
    m_mostSaled->setStyleSheet(recBtnStyle);
    m_mostSaled->setCursor(Qt::PointingHandCursor);
    m_mostSaled->setProperty("active", true);

    m_highScore = new QPushButton("评分最高", m_recommendBar);
    m_highScore->setStyleSheet(recBtnStyle);
    m_highScore->setCursor(Qt::PointingHandCursor);

    m_mostCommented = new QPushButton("最多评价", m_recommendBar);
    m_mostCommented->setStyleSheet(recBtnStyle);
    m_mostCommented->setCursor(Qt::PointingHandCursor);

    barLayout->addWidget(m_mostSaled);
    barLayout->addWidget(m_highScore);
    barLayout->addWidget(m_mostCommented);
    barLayout->addStretch();

    connect(m_mostSaled,     &QPushButton::clicked, this, [this]() { switchRecommendMethod(0); });
    connect(m_highScore,     &QPushButton::clicked, this, [this]() { switchRecommendMethod(1); });
    connect(m_mostCommented, &QPushButton::clicked, this, [this]() { switchRecommendMethod(2); });

    rightLayout->addWidget(m_recommendBar);

    // 菜品滚动区
    auto *scrollArea = new QScrollArea(rightContainer);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea { background: #F5F5F5; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical {
            background: #CCCCCC; border-radius: 3px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #AAAAAA; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");

    m_dishContainer = new QWidget();
    m_dishContainer->setStyleSheet("background: #F5F5F5;");
    m_dishListLayout = new QVBoxLayout(m_dishContainer);
    m_dishListLayout->setContentsMargins(16, 12, 16, 12);
    m_dishListLayout->setSpacing(10);
    m_dishListLayout->addStretch();

    scrollArea->setWidget(m_dishContainer);
    rightLayout->addWidget(scrollArea, 1);

    pageLayout->addWidget(rightContainer, 1);
}

void MenuPage::setData(const QList<Dish_qt> &allItems,
                       const QList<Dish_qt> &bySales,
                       const QList<Dish_qt> &byRating,
                       const QList<Dish_qt> &byComments,
                       const QStringList    &categories)
{
    m_allItems  = allItems;
    m_bySales   = bySales;
    m_byRating  = byRating;
    m_byComments = byComments;

    // 重建分类列表，暂停信号避免触发 refreshDishList
    m_categoryList->blockSignals(true);
    m_categoryList->clear();
    m_categoryList->addItem("全部");
    m_categoryList->addItem("推荐");
    for (const auto &cat : categories)
        m_categoryList->addItem(cat);
    m_categoryList->setCurrentRow(1); // 默认"推荐"
    m_categoryList->blockSignals(false);
}

void MenuPage::setDiscountRate(double rate)
{
    m_discountRate = rate;
    // 用新折扣率重建当前分类的卡片
    refreshDishList(currentCategory());
}

QString MenuPage::currentCategory() const
{
    auto *item = m_categoryList->currentItem();
    return item ? item->text() : "推荐";
}

void MenuPage::onCategoryChanged(int row)
{
    if (row < 0) return;
    m_recommendBar->setVisible(row == 1);
    refreshDishList(m_categoryList->item(row)->text());
}

void MenuPage::switchRecommendMethod(int index)
{
    switch (index) {
        case 0: m_recommendMethod = "销量最高"; break;
        case 1: m_recommendMethod = "评分最高"; break;
        case 2: m_recommendMethod = "最多评价"; break;
        default: break;
    }

    m_mostSaled->setProperty("active",     index == 0);
    m_highScore->setProperty("active",     index == 1);
    m_mostCommented->setProperty("active", index == 2);

    for (auto *btn : {m_mostSaled, m_highScore, m_mostCommented}) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }

    refreshDishList("推荐");
}

void MenuPage::refreshDishList(const QString &category)
{
    // 清除旧卡片
    QLayoutItem *child;
    while ((child = m_dishListLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    int count = 0;

    auto makeCard = [&](const Dish_qt &dish) {
        // 查找该菜品在销量 Top5 / 好评 Top5 中的名次（找不到则为 0）
        int salesRank = 0, ratingRank = 0;
        for (int i = 0; i < m_bySales.size(); i++) {
            if (m_bySales[i].id == dish.id) { salesRank = i + 1; break; }
        }
        for (int i = 0; i < m_byRating.size(); i++) {
            if (m_byRating[i].id == dish.id) { ratingRank = i + 1; break; }
        }
        auto *card = new DishCard(dish, m_discountRate, salesRank, ratingRank, m_dishContainer);
        connect(card, &DishCard::addClicked,     this, [this](int id) { emit addDishClicked(id); });
        connect(card, &DishCard::commentClicked, this, [this](int dishId) {
            for (const auto &d : m_allItems) {
                if (d.id == dishId) {
                    CommentDialog dlg(d, this);
                    dlg.exec();
                    return;
                }
            }
        });
        m_dishListLayout->addWidget(card);
        ++count;
    };

    if (category == "推荐") {
        const QList<Dish_qt> *src = &m_bySales;
        if      (m_recommendMethod == "评分最高") src = &m_byRating;
        else if (m_recommendMethod == "最多评价") src = &m_byComments;
        for (const auto &item : *src) makeCard(item);
    } else {
        for (const auto &item : m_allItems) {
            if (item.category == category || category == "全部")
                makeCard(item);
        }
    }

    m_dishListLayout->addStretch();
    emit dishCountChanged(count);
}
