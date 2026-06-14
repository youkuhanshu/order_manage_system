#include "queue_page.h"

#include <QHBoxLayout>
#include <ctime>

// ================================================================
//  QueuePage
// ================================================================
QueuePage::QueuePage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void QueuePage::setQueueData(int currentCall,
                             const std::vector<QueueMsg> &waiting,
                             const std::vector<QueueMsg> &taking,
                             int myQueueId)
{
    m_currentCall = currentCall;
    m_waiting     = waiting;
    m_taking      = taking;
    m_myQueueId   = myQueueId;
    refreshDisplay();
}

void QueuePage::setupUI()
{
    setStyleSheet("background: #F5F5F5;");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // ---- 顶部叫号横幅 ----
    auto *banner = new QFrame(this);
    banner->setFixedHeight(96);
    banner->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px; border: 1px solid #F0F0F0; }");
    auto *bannerLayout = new QHBoxLayout(banner);
    bannerLayout->setContentsMargins(28, 0, 28, 0);

    auto *callBox = new QVBoxLayout();
    callBox->setSpacing(2);
    auto *callTitle = new QLabel("当前叫号", banner);
    callTitle->setStyleSheet(
        "font-size: 13px; color: #999999; border: none; background: transparent;");
    m_currentCallLabel = new QLabel("--", banner);
    m_currentCallLabel->setStyleSheet(
        "font-size: 38px; font-weight: bold; color: #FF6200;"
        "border: none; background: transparent;");
    callBox->addWidget(callTitle);
    callBox->addWidget(m_currentCallLabel);
    bannerLayout->addLayout(callBox);

    bannerLayout->addStretch();

    m_myStatusLabel = new QLabel("下单后即可在此查看取餐进度", banner);
    m_myStatusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_myStatusLabel->setStyleSheet(
        "font-size: 14px; color: #666666; border: none; background: transparent;");
    bannerLayout->addWidget(m_myStatusLabel);

    mainLayout->addWidget(banner);

    // ---- 两列排队列表 ----
    auto *listsLayout = new QHBoxLayout();
    listsLayout->setSpacing(12);

    // -- 预约排队 --
    auto *waitCard = new QFrame(this);
    waitCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px; border: 1px solid #F0F0F0; }");
    auto *waitLayout = new QVBoxLayout(waitCard);
    waitLayout->setContentsMargins(0, 0, 0, 0);
    waitLayout->setSpacing(0);

    auto *waitHeader = new QLabel("预约排队中", waitCard);
    waitHeader->setFixedHeight(48);
    waitHeader->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #333333; padding-left: 16px;"
        "border: none; border-bottom: 1px solid #F5F5F5; background: transparent;");
    waitLayout->addWidget(waitHeader);

    m_waitingCountLabel = new QLabel("等待 0 桌", waitCard);
    m_waitingCountLabel->setStyleSheet(
        "font-size: 12px; color: #999999; padding: 8px 0 0 16px;"
        "border: none; background: transparent;");
    waitLayout->addWidget(m_waitingCountLabel);

    auto *waitScroll = new QScrollArea(waitCard);
    waitScroll->setWidgetResizable(true);
    waitScroll->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #CCCCCC; border-radius: 3px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: #AAAAAA; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");
    auto *waitContainer = new QWidget();
    waitContainer->setStyleSheet("background: transparent;");
    m_waitingLayout = new QVBoxLayout(waitContainer);
    m_waitingLayout->setContentsMargins(12, 8, 12, 8);
    m_waitingLayout->setSpacing(8);
    m_waitingLayout->addStretch();
    waitScroll->setWidget(waitContainer);
    waitLayout->addWidget(waitScroll, 1);

    listsLayout->addWidget(waitCard, 1);

    // -- 取餐排队 --
    auto *takeCard = new QFrame(this);
    takeCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-radius: 12px; border: 1px solid #F0F0F0; }");
    auto *takeLayout = new QVBoxLayout(takeCard);
    takeLayout->setContentsMargins(0, 0, 0, 0);
    takeLayout->setSpacing(0);

    auto *takeHeader = new QLabel("待取餐", takeCard);
    takeHeader->setFixedHeight(48);
    takeHeader->setStyleSheet(
        "font-size: 15px; font-weight: bold; color: #333333; padding-left: 16px;"
        "border: none; border-bottom: 1px solid #F5F5F5; background: transparent;");
    takeLayout->addWidget(takeHeader);

    m_takingCountLabel = new QLabel("可取餐 0 桌", takeCard);
    m_takingCountLabel->setStyleSheet(
        "font-size: 12px; color: #999999; padding: 8px 0 0 16px;"
        "border: none; background: transparent;");
    takeLayout->addWidget(m_takingCountLabel);

    auto *takeScroll = new QScrollArea(takeCard);
    takeScroll->setWidgetResizable(true);
    takeScroll->setStyleSheet(R"(
        QScrollArea { background: transparent; border: none; }
        QScrollBar:vertical { width: 6px; background: transparent; }
        QScrollBar::handle:vertical { background: #CCCCCC; border-radius: 3px; min-height: 30px; }
        QScrollBar::handle:vertical:hover { background: #AAAAAA; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");
    auto *takeContainer = new QWidget();
    takeContainer->setStyleSheet("background: transparent;");
    m_takingLayout = new QVBoxLayout(takeContainer);
    m_takingLayout->setContentsMargins(12, 8, 12, 8);
    m_takingLayout->setSpacing(8);
    m_takingLayout->addStretch();
    takeScroll->setWidget(takeContainer);
    takeLayout->addWidget(takeScroll, 1);

    listsLayout->addWidget(takeCard, 1);

    mainLayout->addLayout(listsLayout, 1);

    // ---- 底部操作栏 ----
<<<<<<< HEAD
    // 进度无需手动刷新：切到本页、定时叫号、取餐时都会自动刷新
    auto *bottomBar = new QHBoxLayout();
    bottomBar->setSpacing(10);

=======
    auto *bottomBar = new QHBoxLayout();
    bottomBar->setSpacing(10);

    auto *refreshBtn = new QPushButton("刷新进度", this);
    refreshBtn->setFixedHeight(40);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(R"(
        QPushButton {
            background: #0085FF; color: #FFFFFF; border: none;
            border-radius: 8px; font-size: 14px; font-weight: bold; padding: 0 22px;
        }
        QPushButton:hover   { background: #0073E6; }
        QPushButton:pressed { background: #0060BF; }
    )");
    connect(refreshBtn, &QPushButton::clicked, this, &QueuePage::refreshRequested);
    bottomBar->addWidget(refreshBtn);

>>>>>>> 929277828bb293c0d1adb11e36f0a1cd43c54026
    bottomBar->addStretch();

    auto *backBtn = new QPushButton("← 返回菜单", this);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none;"
        "font-size: 13px; color: #AAAAAA; }"
        "QPushButton:hover { color: #0085FF; }");
    connect(backBtn, &QPushButton::clicked, this, &QueuePage::backToMenuRequested);
    bottomBar->addWidget(backBtn);

    mainLayout->addLayout(bottomBar);
}

void QueuePage::refreshDisplay()
{
    // 当前叫号
    if (m_currentCall > 0)
        m_currentCallLabel->setText(QString::number(m_currentCall));
    else
        m_currentCallLabel->setText("--");

    // 我的取餐号状态
    if (m_myQueueId < 0) {
        m_myStatusLabel->setText("下单后即可在此查看取餐进度");
    } else {
        // 判断我的号在哪个队列
        bool inTaking = false;
        for (size_t i = 0; i < m_taking.size(); i++) {
            if (m_taking[i].queue_id == m_myQueueId) { inTaking = true; break; }
        }
        int ahead = -1;
        for (size_t i = 0; i < m_waiting.size(); i++) {
            if (m_waiting[i].queue_id == m_myQueueId) { ahead = (int)i; break; }
        }

        if (inTaking) {
            m_myStatusLabel->setText(
                QString("我的取餐号 %1 · 已叫号，请取餐").arg(m_myQueueId));
        } else if (ahead >= 0) {
            m_myStatusLabel->setText(
                QString("我的取餐号 %1 · 前面还有 %2 桌").arg(m_myQueueId).arg(ahead));
        } else {
            m_myStatusLabel->setText(
                QString("我的取餐号 %1 · 已取餐，感谢惠顾").arg(m_myQueueId));
        }
    }

    // ---- 重建预约排队列表 ----
    QLayoutItem *child;
    while ((child = m_waitingLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    for (size_t i = 0; i < m_waiting.size(); i++) {
        bool isMine = (m_waiting[i].queue_id == m_myQueueId);
        m_waitingLayout->addWidget(makeTicketRow(m_waiting[i], (int)i, isMine, false));
    }
    if (m_waiting.empty()) {
        auto *empty = new QLabel("暂无排队", this);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(
            "font-size: 13px; color: #CCCCCC; padding: 20px;"
            "border: none; background: transparent;");
        m_waitingLayout->addWidget(empty);
    }
    m_waitingLayout->addStretch();
    m_waitingCountLabel->setText(QString("等待 %1 桌").arg((int)m_waiting.size()));

    // ---- 重建取餐排队列表 ----
    while ((child = m_takingLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    for (size_t i = 0; i < m_taking.size(); i++) {
        bool isMine = (m_taking[i].queue_id == m_myQueueId);
        m_takingLayout->addWidget(makeTicketRow(m_taking[i], (int)i, isMine, true));
    }
    if (m_taking.empty()) {
        auto *empty = new QLabel("暂无可取餐", this);
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet(
            "font-size: 13px; color: #CCCCCC; padding: 20px;"
            "border: none; background: transparent;");
        m_takingLayout->addWidget(empty);
    }
    m_takingLayout->addStretch();
    m_takingCountLabel->setText(QString("可取餐 %1 桌").arg((int)m_taking.size()));
}

QFrame *QueuePage::makeTicketRow(const QueueMsg &msg, int position, bool isMine, bool ready)
{
    auto *row = new QFrame();
    row->setObjectName("queueRow");
    // 自己的号高亮，普通号白底
    QString border = isMine ? "#FF6200" : "#F5F5F5";
    QString bg     = isMine ? "#FFF8F3" : "#FFFFFF";
    row->setStyleSheet(QString(
        "#queueRow { background: %1; border-radius: 8px; border: 1px solid %2; }")
        .arg(bg).arg(border));
    row->setFixedHeight(64);
    row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(12, 8, 12, 8);
    rowLayout->setSpacing(12);

    // 号码圆牌
    QString bubbleColor = ready ? "#58D68D" : "#5DADE2";
    if (isMine) bubbleColor = "#FF6200";
    auto *bubble = new QFrame(row);
    bubble->setFixedSize(42, 42);
    bubble->setStyleSheet(QString("QFrame { background: %1; border-radius: 8px; }")
        .arg(bubbleColor));
    auto *bubbleLayout = new QVBoxLayout(bubble);
    bubbleLayout->setContentsMargins(0, 0, 0, 0);
    auto *bubbleText = new QLabel(QString::number(msg.queue_id), bubble);
    bubbleText->setAlignment(Qt::AlignCenter);
    bubbleText->setStyleSheet(
        "color: #FFFFFF; font-size: 15px; font-weight: bold;"
        "border: none; background: transparent;");
    bubbleLayout->addWidget(bubbleText);
    rowLayout->addWidget(bubble);

    // 中间信息
    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);

    QString title = QString("取餐号 %1").arg(msg.queue_id);
    if (isMine) title += "（我的）";
    auto *titleLabel = new QLabel(title, row);
    titleLabel->setStyleSheet(
        "font-size: 14px; font-weight: 600; color: #333333;"
        "border: none; background: transparent;");
    infoLayout->addWidget(titleLabel);

    // 等待时长
    long mins = (long)((std::time(nullptr) - msg.in_time) / 60);
    QString waitText;
    if (mins <= 0)      waitText = "刚刚下单";
    else if (mins < 60) waitText = QString("已等待 %1 分钟").arg(mins);
    else                waitText = QString("已等待 %1 小时").arg(mins / 60);

    auto *subLabel = new QLabel(
        QString("订单 #%1 · %2").arg(msg.order_id).arg(waitText), row);
    subLabel->setStyleSheet(
        "font-size: 11px; color: #AAAAAA; border: none; background: transparent;");
    infoLayout->addWidget(subLabel);

    rowLayout->addLayout(infoLayout, 1);

    // 右侧状态
<<<<<<< HEAD
    if (ready) {
        // 「取餐」按钮：点击后由主窗口弹出评价窗并把该号移出取餐队列
        auto *takeBtn = new QPushButton("取餐", row);
=======
    if (ready && isMine) {
        // 自己的取餐号 → 可点击的「请取餐」按钮，点击后弹出评价
        auto *takeBtn = new QPushButton("请取餐", row);
>>>>>>> 929277828bb293c0d1adb11e36f0a1cd43c54026
        takeBtn->setCursor(Qt::PointingHandCursor);
        takeBtn->setFixedHeight(28);
        takeBtn->setStyleSheet(R"(
            QPushButton {
                font-size: 12px; color: #FFFFFF; background: #FF6200;
<<<<<<< HEAD
                border: none; border-radius: 14px; padding: 0 18px; font-weight: bold;
            }
            QPushButton:hover   { background: #E55A00; }
            QPushButton:pressed { background: #CC5000; }
        )");
        const int qid = msg.queue_id;
        connect(takeBtn, &QPushButton::clicked, this, [this, qid]() {
            emit pickupRequested(qid);
        });
        rowLayout->addWidget(takeBtn);
=======
                border-radius: 14px; border: none; padding: 0 16px;
                font-weight: bold;
            }
            QPushButton:hover   { background: #E55A00; }
            QPushButton:pressed { background: #CC4F00; }
        )");
        int qid = msg.queue_id;
        connect(takeBtn, &QPushButton::clicked, this, [this, qid]() {
            emit mealTakenRequested(qid);
        });
        rowLayout->addWidget(takeBtn);
    } else if (ready) {
        auto *pill = new QLabel("请取餐", row);
        pill->setAlignment(Qt::AlignCenter);
        pill->setFixedHeight(24);
        pill->setStyleSheet(
            "font-size: 12px; color: #FFFFFF; background: #FF6200;"
            "border-radius: 12px; padding: 0 12px;");
        rowLayout->addWidget(pill);
>>>>>>> 929277828bb293c0d1adb11e36f0a1cd43c54026
    } else {
        auto *aheadLabel = new QLabel(
            position == 0 ? "即将叫号" : QString("前面 %1 桌").arg(position), row);
        aheadLabel->setStyleSheet(
            "font-size: 12px; color: #888888; border: none; background: transparent;");
        rowLayout->addWidget(aheadLabel);
    }

    return row;
}
