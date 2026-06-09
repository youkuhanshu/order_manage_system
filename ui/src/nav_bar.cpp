#include "nav_bar.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QStyle>

NavBar::NavBar(QWidget *parent) : QFrame(parent)
{
    setupUI();
}

void NavBar::setupUI()
{
    setFixedHeight(54);
    setObjectName("navBar");
    setStyleSheet("QFrame#navBar { background: #FFFFFF; border-bottom: 1px solid #E8E8E8; }");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(0);

    // ---- 用户信息区域（可点击弹出菜单）----
    m_userPanel = new QFrame(this);
    m_userPanel->setObjectName("userPanel");
    m_userPanel->setCursor(Qt::PointingHandCursor);
    m_userPanel->setStyleSheet(R"(
        #userPanel { background: transparent; border: none; border-radius: 8px; }
        #userPanel:hover { background: #F5F5F5; }
    )");
    m_userPanel->installEventFilter(this);

    auto *panelLayout = new QHBoxLayout(m_userPanel);
    panelLayout->setContentsMargins(8, 0, 8, 0);
    panelLayout->setSpacing(8);

    m_userAvatarLabel = new QLabel(m_userPanel);
    m_userAvatarLabel->setFixedSize(34, 34);
    m_userAvatarLabel->setAlignment(Qt::AlignCenter);
    m_userAvatarLabel->setStyleSheet(
        "background: #CCCCCC; border-radius: 17px;"
        "color: #FFFFFF; font-size: 14px; font-weight: bold; border: none;");

    auto *textLayout = new QVBoxLayout();
    textLayout->setSpacing(1);
    textLayout->setAlignment(Qt::AlignVCenter);

    m_userNameLabel = new QLabel(m_userPanel);
    m_userNameLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #333333;"
        "border: none; background: transparent;");

    m_userLevelLabel = new QLabel(m_userPanel);
    m_userLevelLabel->setStyleSheet(
        "font-size: 11px; color: #AAAAAA; border: none; background: transparent;");

    textLayout->addWidget(m_userNameLabel);
    textLayout->addWidget(m_userLevelLabel);
    panelLayout->addWidget(m_userAvatarLabel);
    panelLayout->addLayout(textLayout);

    layout->addWidget(m_userPanel);
    layout->addSpacing(20);

    // ---- 导航按钮 ----
    const QString navBtnStyle = R"(
        QPushButton {
            background: transparent; border: none;
            font-size: 14px; color: #666666; padding: 6px 18px;
        }
        QPushButton:hover { color: #0085FF; }
        QPushButton[active="true"] {
            color: #0085FF; font-weight: bold;
            border-bottom: 2px solid #0085FF;
        }
    )";

    m_btnMenu = new QPushButton("菜品菜单", this);
    m_btnMenu->setStyleSheet(navBtnStyle);
    m_btnMenu->setCursor(Qt::PointingHandCursor);
    connect(m_btnMenu, &QPushButton::clicked, this, [this]() { emit navClicked(2); });

    m_btnCart = new QPushButton("购物车", this);
    m_btnCart->setStyleSheet(navBtnStyle);
    m_btnCart->setCursor(Qt::PointingHandCursor);
    connect(m_btnCart, &QPushButton::clicked, this, [this]() { emit navClicked(3); });

    m_btnQueue = new QPushButton("排队进度", this);
    m_btnQueue->setStyleSheet(navBtnStyle);
    m_btnQueue->setCursor(Qt::PointingHandCursor);
    connect(m_btnQueue, &QPushButton::clicked, this, [this]() { emit navClicked(4); });

    layout->addWidget(m_btnMenu);
    layout->addWidget(m_btnCart);
    layout->addWidget(m_btnQueue);
    layout->addStretch();

    // ---- 菜品数量 ----
    m_dishCountLabel = new QLabel(this);
    m_dishCountLabel->setStyleSheet(
        "font-size: 13px; color: #999999; border: none; background: transparent;");
    layout->addWidget(m_dishCountLabel);
}

void NavBar::setUser(const User &user)
{
    static const char *avatarColors[] = {
        "#FF6B4A", "#5DADE2", "#58D68D",
        "#AF7AC5", "#F4D03F", "#EB984E"
    };

    const QString name = QString::fromStdString(user.name);
    int colorIdx = name.isEmpty() ? 0 : (name[0].unicode() % 6);

    m_userAvatarLabel->setText(name.isEmpty() ? "?" : QString(name[0]));
    m_userAvatarLabel->setStyleSheet(QString(
        "background: %1; border-radius: 17px;"
        "color: #FFFFFF; font-size: 14px; font-weight: bold; border: none;")
        .arg(avatarColors[colorIdx]));

    m_userNameLabel->setText(name);

    QString levelText, levelColor;
    if      (user.level == "PLATINUM") { levelText = "白金会员"; levelColor = "#9C4FE8"; }
    else if (user.level == "GOLD")     { levelText = "黄金会员"; levelColor = "#E8960A"; }
    else if (user.level == "SILVER")   { levelText = "白银会员"; levelColor = "#7A9BB5"; }
    else                               { levelText = "普通用户"; levelColor = "#AAAAAA"; }

    m_userLevelLabel->setText(levelText);
    m_userLevelLabel->setStyleSheet(
        QString("font-size: 11px; color: %1; border: none; background: transparent;")
        .arg(levelColor));
}

void NavBar::setDishCount(int count)
{
    m_dishCountLabel->setText(QString("共 %1 道菜品").arg(count));
}

void NavBar::setActiveNav(int pageIndex)
{
    m_btnMenu->setProperty("active", pageIndex == 2);
    m_btnCart->setProperty("active", pageIndex == 3);
    m_btnQueue->setProperty("active", pageIndex == 4);

    for (auto *btn : {m_btnMenu, m_btnCart, m_btnQueue}) {
        btn->style()->unpolish(btn);
        btn->style()->polish(btn);
    }

    m_dishCountLabel->setVisible(pageIndex == 2);
}

bool NavBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_userPanel && event->type() == QEvent::MouseButtonPress) {
        QMenu menu(this);
        menu.setStyleSheet(R"(
            QMenu {
                background: #FFFFFF; border: 1px solid #E8E8E8;
                border-radius: 8px; padding: 4px 0;
            }
            QMenu::item { padding: 10px 28px; font-size: 14px; color: #333333; }
            QMenu::item:selected { background: #FFF0EE; color: #FF4D2E; }
        )");

        connect(menu.addAction("退出登录"), &QAction::triggered,
                this, [this]() { emit logoutRequested(); });

        menu.exec(m_userPanel->mapToGlobal(QPoint(0, m_userPanel->height())));
        return true;
    }
    return QFrame::eventFilter(obj, event);
}
