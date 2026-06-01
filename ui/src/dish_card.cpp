#include "dish_card.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

// ---- 分类色板 --------------------------------------------------
static const char *categoryColor(const QString &category)
{
    if (category == "热菜") return "#FF6B35";
    if (category == "主食") return "#FFC300";
    if (category == "饮品") return "#4ECDC4";
    if (category == "甜品") return "#FF8C94";
    return "#BBBBBB";
}

// ================================================================
//  DishCard
// ================================================================
DishCard::DishCard(const Dish_qt &dish, QWidget *parent)
    : QFrame(parent), m_dish(dish)
{
    setupUI(dish);
}

void DishCard::setupUI(const Dish_qt &dish)
{
    // ---- 卡片容器 ----
    setObjectName("dishCard");
    setStyleSheet(R"(
        #dishCard {
            background: #FFFFFF;
            border-radius: 10px;
            border: 1px solid #F0F0F0;
        }
        #dishCard:hover {
            border: 1px solid #FFD666;
            background: #FFFEFA;
        }
    )");
    setFixedHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(14, 12, 14, 12);
    mainLayout->setSpacing(14);

    // ---- 左侧：图片占位 ----
    m_imageFrame = new QFrame(this);
    m_imageFrame->setFixedSize(94, 94);
    m_imageFrame->setStyleSheet(QString(R"(
        QFrame {
            background: %1;
            border-radius: 10px;
        }
    )").arg(categoryColor(dish.category)));

    auto *imgLayout = new QVBoxLayout(m_imageFrame);
    imgLayout->setAlignment(Qt::AlignCenter);
    imgLayout->setContentsMargins(0, 0, 0, 0);

    m_imageText = new QLabel(dish.name.at(0), m_imageFrame);
    m_imageText->setAlignment(Qt::AlignCenter);
    m_imageText->setStyleSheet(
        "color: #FFFFFF; font-size: 30px; font-weight: bold;"
        "background: transparent; border: none;");
    imgLayout->addWidget(m_imageText);

    mainLayout->addWidget(m_imageFrame);

    // ---- 中间：菜品信息 ----
    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    m_nameLabel = new QLabel(dish.name, this);
    m_nameLabel->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #333333;"
        "border: none; background: transparent;");
    infoLayout->addWidget(m_nameLabel);

    m_descLabel = new QLabel(dish.description, this);
    m_descLabel->setStyleSheet(
        "font-size: 12px; color: #999999;"
        "border: none; background: transparent;");
    m_descLabel->setMaximumWidth(360);
    infoLayout->addWidget(m_descLabel);

    infoLayout->addStretch();

    // 评分 + 销量
    auto *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(10);

    m_ratingLabel = new QLabel(QString("★ %1").arg(dish.rating, 0, 'f', 1), this);
    m_ratingLabel->setStyleSheet(
        "font-size: 12px; color: #FFC300; font-weight: bold;"
        "border: none; background: transparent;");
    statsLayout->addWidget(m_ratingLabel);

    m_salesLabel = new QLabel(QString("月售 %1").arg(dish.sales), this);
    m_salesLabel->setStyleSheet(
        "font-size: 11px; color: #BBBBBB;"
        "border: none; background: transparent;");
    statsLayout->addWidget(m_salesLabel);

    statsLayout->addStretch();
    infoLayout->addLayout(statsLayout);

    mainLayout->addLayout(infoLayout, 1);

    // ---- 右侧：价格 & 加购按钮 ----
    auto *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->setSpacing(10);

    m_priceLabel = new QLabel(QString("¥%1").arg(dish.price, 0, 'f', 0), this);
    m_priceLabel->setStyleSheet(
        "font-size: 22px; font-weight: bold; color: #FF4D00;"
        "border: none; background: transparent;");
    m_priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->addWidget(m_priceLabel);

    m_addBtn = new QPushButton("+", this);
    m_addBtn->setFixedSize(30, 30);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setStyleSheet(R"(
        QPushButton {
            background: #FFC300;
            color: #FFFFFF;
            border: none;
            border-radius: 15px;
            font-size: 20px;
            font-weight: bold;
        }
        QPushButton:hover  { background: #FFB800; }
        QPushButton:pressed { background: #E5A800; }
    )");
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        emit addClicked(m_dish.id);
    });
    rightLayout->addWidget(m_addBtn, 0, Qt::AlignRight);

    mainLayout->addLayout(rightLayout);
}
