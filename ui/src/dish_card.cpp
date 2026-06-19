#include "dish_card.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

// 分类色板 
static const char *categoryColor(const QString &category)
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

//  DishCard：菜品，折扣率，销量榜单，评分榜单
DishCard::DishCard(const Dish_qt &dish, double discountRate,
                   int salesRank, int ratingRank,
                   QWidget *parent)
    : QFrame(parent), m_dish(dish), m_discountRate(discountRate),
      m_salesRank(salesRank), m_ratingRank(ratingRank),
      m_memberPriceLabel(nullptr)
{
    setupUI(dish);
}

void DishCard::setupUI(const Dish_qt &dish)
{
    // 卡片容器
    setObjectName("dishCard");
    setStyleSheet(R"(
        #dishCard {
            background: #FFFFFF;
            border-radius: 10px;
            border: 1px solid #F0F0F0;
        }
        #dishCard:hover {
            border: 1px solid #e0e0e0;
            background: #fafdff;
        }
    )");
    setFixedHeight(m_discountRate < 1.0 ? 130 : 120); // 有会员折扣时需要高一点，显示折扣价
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // 宽度随窗口变化，高度锁死

    // 整个菜品卡片的布局
    auto *mainLayout = new QHBoxLayout(this); // 水平排列布局
    mainLayout->setContentsMargins(14, 12, 14, 12); // 左 上 右 下，边框宽度
    mainLayout->setSpacing(14); // 相邻控件之间距离为14像素

    // 画一个图片
    m_imageFrame = new QFrame(this); // 基础矩形容器
    m_imageFrame->setFixedSize(94, 94);
    m_imageFrame->setStyleSheet(QString(R"(
        QFrame {
            background: %1;
            border-radius: 10px;
        }
    )").arg(categoryColor(dish.category))); // 根据菜品类型设置背景色

    // 图片的布局
    auto *imgLayout = new QVBoxLayout(m_imageFrame);
    imgLayout->setAlignment(Qt::AlignCenter); // 所有子控件均居中显示
    imgLayout->setContentsMargins(0, 0, 0, 0);

    m_imageText = new QLabel(dish.name.at(0), m_imageFrame); // 标签显示菜品名第一个字，父控件为imageFrame
    m_imageText->setAlignment(Qt::AlignCenter);
    m_imageText->setStyleSheet(
        "color: #FFFFFF; font-size: 30px; font-weight: bold;"
        "background: transparent; border: none;");
    imgLayout->addWidget(m_imageText); // 文字加入图片布局

    mainLayout->addWidget(m_imageFrame); // 图片布局加入菜品卡片布局

    // 中间：菜品信息
    auto *infoLayout = new QVBoxLayout(); // 菜品信息布局
    infoLayout->setSpacing(4); // 垂直排列，也就是高度间隔4个像素

    // 菜名 + 排行榜徽章（同一行）
    auto *nameRow = new QHBoxLayout();
    nameRow->setSpacing(8);
    nameRow->setContentsMargins(0, 0, 0, 0);

    m_nameLabel = new QLabel(dish.name, this);
    m_nameLabel->setStyleSheet(
        "font-size: 16px; font-weight: 600; color: #333333;"
        "border: none; background: transparent;");
    nameRow->addWidget(m_nameLabel); // 菜名/排行榜布局加入菜名

    auto makeBadge = [&](const QString &text, const char *bg) {
        auto *badge = new QLabel(text, this);
        badge->setStyleSheet(QString(
            "font-size: 10px; color: #FFFFFF; background: %1;"
            "border-radius: 8px; padding: 1px 7px; border: none;").arg(bg));
        badge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        nameRow->addWidget(badge);
    };
    if (m_salesRank > 0)  makeBadge(QString("本店销量第%1").arg(m_salesRank), "#FF6200");
    if (m_ratingRank > 0) makeBadge(QString("好评榜第%1").arg(m_ratingRank),  "#52C41A");
    nameRow->addStretch(); // 加一个弹簧，让菜名 + 排行榜居于上方

    infoLayout->addLayout(nameRow);

    m_descLabel = new QLabel(dish.description, this);
    m_descLabel->setStyleSheet(
        "font-size: 12px; color: #999999;"
        "border: none; background: transparent;");
    m_descLabel->setMaximumWidth(360);
    infoLayout->addWidget(m_descLabel);

    infoLayout->addStretch(); // 菜品信息整体居左显示

    // 评分 + 销量
    auto *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(10);

    m_ratingLabel = new QLabel(QString("★ %1").arg(dish.rating, 0, 'f', 1), this);
    m_ratingLabel->setStyleSheet(
        "font-size: 12px; color: #ffd500; font-weight: bold;"
        "border: none; background: transparent;");
    statsLayout->addWidget(m_ratingLabel);

    m_salesLabel = new QLabel(QString("月售 %1").arg(dish.sales), this);
    m_salesLabel->setStyleSheet(
        "font-size: 11px; color: #BBBBBB;"
        "border: none; background: transparent;");
    statsLayout->addWidget(m_salesLabel);

    auto *commentBtn = new QPushButton("查看评论 ›", this);
    commentBtn->setCursor(Qt::PointingHandCursor); // 鼠标停靠时改变样式
    commentBtn->setFocusPolicy(Qt::NoFocus); // 不接受焦点
    commentBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 11px; color: #AAAAAA; padding: 0; }"
        "QPushButton:hover { color: #0085FF; }");
    connect(commentBtn, &QPushButton::clicked, this, [this]() {
        emit commentClicked(m_dish.id);
    }); // 点击时发送commentClicked信号，传递m_dish.id参数
    statsLayout->addWidget(commentBtn);
    statsLayout->addStretch(); // 居左显示
    infoLayout->addLayout(statsLayout);

    mainLayout->addLayout(infoLayout, 1);

    // 右侧：价格 & 加购按钮
    auto *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter); // 右侧居中
    rightLayout->setSpacing(4);

    if (m_discountRate < 1.0) {
        // 原价：灰色 + 删除线
        m_priceLabel = new QLabel(QString("¥%1").arg(dish.price, 0, 'f', 0), this);
        m_priceLabel->setStyleSheet(
            "font-size: 14px; color: #AAAAAA; text-decoration: line-through;"
            "border: none; background: transparent;");
        m_priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rightLayout->addWidget(m_priceLabel);

        // 会员价：金橙色
        double memberPrice = dish.price * m_discountRate;
        m_memberPriceLabel = new QLabel(
            QString("会员价：¥%1").arg(memberPrice, 0, 'f', 0), this);
        m_memberPriceLabel->setStyleSheet(
            "font-size: 17px; font-weight: bold; color: #E8960A;"
            "border: none; background: transparent;");
        m_memberPriceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rightLayout->addWidget(m_memberPriceLabel);
    }
    else {
        // 普通价格
        m_priceLabel = new QLabel(QString("¥%1").arg(dish.price, 0, 'f', 0), this);
        m_priceLabel->setStyleSheet(
            "font-size: 18px; font-weight: normal; color: #FF4D2E;"
            "border: none; background: transparent;");
        m_priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rightLayout->addWidget(m_priceLabel);
    }

    rightLayout->addSpacing(6);
    m_addBtn = new QPushButton("+", this);
    m_addBtn->setFixedSize(30, 30);
    m_addBtn->setCursor(Qt::PointingHandCursor);
    m_addBtn->setFocusPolicy(Qt::NoFocus);
    m_addBtn->setStyleSheet(R"(
        QPushButton {
            background: #ffd500;
            color: #FFFFFF;
            border: none;
            border-radius: 15px;
            font-size: 20px;
            font-weight: bold;
        }
        QPushButton:hover  { background: #e6b000; }
        QPushButton:pressed { background: #daa401; }
    )");
    connect(m_addBtn, &QPushButton::clicked, this, [this]() {
        emit addClicked(m_dish.id);
    }); // 发送addClicked信号，传递m_dish.id
    rightLayout->addWidget(m_addBtn, 0, Qt::AlignRight);

    mainLayout->addLayout(rightLayout);
}
