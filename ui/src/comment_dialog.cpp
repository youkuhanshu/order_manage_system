#include "comment_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QDateTime>
#include <QProgressBar>
#include <QMessageBox>
#include <algorithm>

std::vector<User> CommentDialog::m_users;

CommentDialog::CommentDialog(const Dish_qt &dish, QWidget *parent) : QDialog(parent)
{
    std::vector<CommentMsg> allComments;

    // 加载用户、评论信息
    FileManager fileManager;
    fileManager.LoadUsers();
    fileManager.LoadComments();
    allComments = fileManager.getComments();
    m_users = fileManager.getUsers_cpp();

    if (m_users.size() == 0) {
        QMessageBox::warning(this, "加载失败", QString("未能加载用户数据\n请确认文件存在且格式正确!"));
    }
    if (allComments.size() == 0) {
        QMessageBox::warning(this, "加载失败", QString("未能加载评论数据\n请确认文件存在且格式正确!"));
    }

    // 筛选该菜品的评论
    const std::string idStr = std::to_string(dish.id);
    for (const auto &c : allComments) {
        for (const auto &did : c.dish_ids) {
            if (did == idStr) {
                m_comments.append(c);
                break;
            }
        }
    }

    setupUI(dish, m_comments);
}

void CommentDialog::setupUI(const Dish_qt &dish, const QList<CommentMsg> &comments)
{
    setWindowTitle(QString("用户评价 · %1").arg(dish.name));
    setMinimumWidth(520);
    setMaximumWidth(600);
    setStyleSheet("QDialog { background: #F5F5F5; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ---- 顶部标题栏 ----
    auto *titleBar = new QFrame(this);
    titleBar->setFixedHeight(52);
    titleBar->setStyleSheet(
        "QFrame { background: #FFFFFF; border-bottom: 1px solid #E8E8E8; }");
    auto *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 16, 0);

    auto *titleLabel = new QLabel(
        QString("用户评价 · <span style='color:#333333;font-weight:bold;'>%1</span>")
            .arg(dish.name),
        titleBar);
    titleLabel->setStyleSheet(
        "font-size: 15px; color: #666666; border: none; background: transparent;");
    titleLayout->addWidget(titleLabel, 1);

    // 排序切换
    m_sortCombo = new QComboBox(titleBar);
    m_sortCombo->addItem("按时间排序");
    m_sortCombo->addItem("按评分排序");
    m_sortCombo->setCurrentIndex(m_sortMode);
    m_sortCombo->setStyleSheet(R"(
        QComboBox {
            background: #FFFFFF; border: 1px solid #E0E0E0; border-radius: 6px;
            font-size: 12px; color: #666666; padding: 4px 8px;
            min-width: 90px;
        }
        QComboBox:hover { border-color: #0085FF; }
        QComboBox::drop-down { border: none; width: 20px; }
        QComboBox QAbstractItemView {
            background: #FFFFFF; border: 1px solid #E0E0E0;
            selection-background-color: #EAF4FF; selection-color: #0085FF;
            font-size: 12px; color: #666666;
        }
    )");
    connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CommentDialog::onSortChanged);
    titleLayout->addWidget(m_sortCombo);

    auto *closeBtn = new QPushButton("✕", titleBar);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent; border: none;
            font-size: 14px; color: #AAAAAA; border-radius: 14px;
        }
        QPushButton:hover { background: #F0F0F0; color: #666666; }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    titleLayout->addWidget(closeBtn);

    mainLayout->addWidget(titleBar);

    // ---- 评分概览卡片 ----
    auto *summaryCard = new QFrame(this);
    summaryCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-bottom: 1px solid #F0F0F0; }");
    auto *summaryLayout = new QHBoxLayout(summaryCard);
    summaryLayout->setContentsMargins(20, 16, 20, 16);
    summaryLayout->setSpacing(24);

    // 大评分数字
    auto *scoreBox = new QVBoxLayout();
    scoreBox->setAlignment(Qt::AlignCenter);
    scoreBox->setSpacing(2);

    double avgScore = 0.0;
    if (!comments.isEmpty()) {
        for (const auto &c : comments) avgScore += c.rate;
        avgScore /= comments.size();
    } else {
        avgScore = dish.rating;
    }

    auto *scoreBig = new QLabel(QString::number(avgScore, 'f', 1), summaryCard);
    scoreBig->setAlignment(Qt::AlignCenter);
    scoreBig->setStyleSheet(
        "font-size: 42px; font-weight: bold; color: #FF6B35;"
        "border: none; background: transparent;");
    scoreBox->addWidget(scoreBig);

    auto *starRow = new QLabel(summaryCard);
    QString stars;
    int fullStars = (int)avgScore;
    for (int i = 0; i < 5; i++) stars += (i < fullStars ? "★" : "☆");
    starRow->setText(stars);
    starRow->setAlignment(Qt::AlignCenter);
    starRow->setStyleSheet(
        "font-size: 16px; color: #ffd500; letter-spacing: 2px;"
        "border: none; background: transparent;");
    scoreBox->addWidget(starRow);

    auto *countLabel = new QLabel(
        QString("%1 条评价").arg(comments.size()), summaryCard);
    countLabel->setAlignment(Qt::AlignCenter);
    countLabel->setStyleSheet(
        "font-size: 12px; color: #AAAAAA; border: none; background: transparent;");
    scoreBox->addWidget(countLabel);

    summaryLayout->addLayout(scoreBox);

    // 分隔线
    auto *divider = new QFrame(summaryCard);
    divider->setFrameShape(QFrame::VLine);
    divider->setStyleSheet("color: #F0F0F0;");
    summaryLayout->addWidget(divider);

    // 星级分布条
    auto *barsLayout = new QVBoxLayout();
    barsLayout->setSpacing(5);
    int total = comments.size();
    int dist[6] = {0};
    for (const auto &c : comments) {
        if (c.rate >= 1 && c.rate <= 5) dist[c.rate]++;
    }
    for (int star = 5; star >= 1; star--) {
        barsLayout->addWidget(makeRatingBar(star, dist[star], total, summaryCard));
    }
    summaryLayout->addLayout(barsLayout, 1);

    mainLayout->addWidget(summaryCard);

    // ---- 评论列表 ----
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            background: #F5F5F5;
            border: none;
        }
        QScrollBar:vertical {
            width: 6px; background: transparent;
        }
        QScrollBar::handle:vertical {
            background: #CCCCCC; border-radius: 3px; min-height: 30px;
        }
        QScrollBar::handle:vertical:hover { background: #AAAAAA; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
    )");

    m_listContainer = new QWidget();
    m_listContainer->setStyleSheet("background: #F5F5F5;");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(12, 12, 12, 12);
    m_listLayout->setSpacing(8);

    if (comments.isEmpty()) {
        auto *emptyLabel = new QLabel("暂无评论~", m_listContainer);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(
            "font-size: 14px; color: #CCCCCC; padding: 40px;"
            "border: none; background: transparent;");
        m_listLayout->addWidget(emptyLabel);
    } else {
        for (const auto &c : comments) {
            m_listLayout->addWidget(makeCommentCard(c, m_listContainer));
        }
    }
    m_listLayout->addStretch();

    scrollArea->setWidget(m_listContainer);
    mainLayout->addWidget(scrollArea, 1);

    resize(540, 560);
}

// 排序模式切换
void CommentDialog::onSortChanged(int index)
{
    if (index == m_sortMode) return;
    m_sortMode = index;

    // 按所选模式排序
    if (m_sortMode == 0) {
        // 按时间排序（降序：最新在前）
        std::sort(m_comments.begin(), m_comments.end(),
                  [](const CommentMsg &a, const CommentMsg &b) {
                      return a.in_time > b.in_time;
                  });
    } else {
        // 按评分排序（降序：高分在前）
        std::sort(m_comments.begin(), m_comments.end(),
                  [](const CommentMsg &a, const CommentMsg &b) {
                      return a.rate > b.rate;
                  });
    }

    refreshCommentList();
}

// 清空并重建评论卡片列表
void CommentDialog::refreshCommentList()
{
    // 清除旧卡片
    QLayoutItem *child;
    while ((child = m_listLayout->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }

    if (m_comments.isEmpty()) {
        auto *emptyLabel = new QLabel("暂无评论~", m_listContainer);
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet(
            "font-size: 14px; color: #CCCCCC; padding: 40px;"
            "border: none; background: transparent;");
        m_listLayout->addWidget(emptyLabel);
    } else {
        for (const auto &c : m_comments) {
            m_listLayout->addWidget(makeCommentCard(c, m_listContainer));
        }
    }
    m_listLayout->addStretch();
}

// 单条评论卡片
QWidget *CommentDialog::makeCommentCard(const CommentMsg &comment, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName("commentCard");
    card->setStyleSheet(R"(
        #commentCard {
            background: #FFFFFF;
            border-radius: 10px;
            border: 1px solid #F0F0F0;
        }
    )");

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);

    // 顶行：头像 + 用户名 + 星级 + 时间
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(10);

    // 头像（圆形色块）
    auto *avatar = new QFrame(card);
    avatar->setFixedSize(34, 34);
    static const char *avatarColors[] = {
        "#FF6B4A",
        "#5DADE2",
        "#58D68D",
        "#AF7AC5",
        "#F4D03F",
        "#EB984E"
    };
    int colorIdx = 0;
    if (!comment.user_id.empty()) colorIdx = (comment.user_id[0]) % 6;
    avatar->setStyleSheet(QString(
        "QFrame { background: %1; border-radius: 17px; }"
    ).arg(avatarColors[colorIdx]));

    auto *avatarLayout = new QVBoxLayout(avatar);
    avatarLayout->setContentsMargins(0, 0, 0, 0);
    QString initChar = comment.user_id.empty() ? "?" : matchIDName(comment.user_id);
    auto *initLabel = new QLabel(initChar[0], avatar);
    initLabel->setAlignment(Qt::AlignCenter);
    initLabel->setStyleSheet(
        "color: #FFFFFF; font-size: 14px; font-weight: bold;"
        "background: transparent; border: none;");
    avatarLayout->addWidget(initLabel);
    topRow->addWidget(avatar);

    // 用户名 + 星级（竖排）
    auto *nameStarLayout = new QVBoxLayout();
    nameStarLayout->setSpacing(2);

    auto *nameLabel = new QLabel(matchIDName(comment.user_id), card);
    nameLabel->setStyleSheet(
        "font-size: 13px; font-weight: 600; color: #333333;"
        "border: none; background: transparent;");
    nameStarLayout->addWidget(nameLabel);

    auto *starsLabel = new QLabel(starStr(comment.rate), card);
    starsLabel->setStyleSheet(
        "font-size: 12px; color: #ffd500; border: none; background: transparent;");
    nameStarLayout->addWidget(starsLabel);

    topRow->addLayout(nameStarLayout, 1);

    auto *timeLabel = new QLabel(formatTime(comment.in_time), card);
    timeLabel->setStyleSheet(
        "font-size: 11px; color: #CCCCCC; border: none; background: transparent;");
    topRow->addWidget(timeLabel);

    layout->addLayout(topRow);

    // 评论正文
    auto *textLabel = new QLabel(
        QString::fromStdString(comment.comment), card);
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet(
        "font-size: 13px; color: #555555; line-height: 1.6;"
        "border: none; background: transparent;");
    layout->addWidget(textLabel);

    return card;
}

// 星级分布条
QFrame *CommentDialog::makeRatingBar(int star, int count, int total, QWidget *parent)
{
    auto *row = new QFrame(parent);
    row->setStyleSheet("QFrame { background: transparent; border: none; }");
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(6);

    auto *starLabel = new QLabel(QString("%1★").arg(star), row);
    starLabel->setFixedWidth(22);
    starLabel->setStyleSheet(
        "font-size: 11px; color: #ffd500; border: none; background: transparent;");
    rowLayout->addWidget(starLabel);

    auto *bar = new QProgressBar(row);
    bar->setRange(0, qMax(total, 1));
    bar->setValue(count);
    bar->setFixedHeight(6);
    bar->setTextVisible(false);
    bar->setStyleSheet(R"(
        QProgressBar {
            background: #F0F0F0; border-radius: 3px; border: none;
        }
        QProgressBar::chunk {
            background: #ffd500; border-radius: 3px;
        }
    )");
    rowLayout->addWidget(bar, 1);

    auto *cntLabel = new QLabel(QString::number(count), row);
    cntLabel->setFixedWidth(18);
    cntLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cntLabel->setStyleSheet(
        "font-size: 11px; color: #AAAAAA; border: none; background: transparent;");
    rowLayout->addWidget(cntLabel);

    return row;
}

// 时间戳 → "MM-dd HH:mm"
QString CommentDialog::formatTime(std::time_t t)
{
    return QDateTime::fromSecsSinceEpoch((qint64)t)
               .toString("MM-dd HH:mm");
}

// 评分 → 星号字符串
QString CommentDialog::starStr(int rate)
{
    QString s;
    for (int i = 0; i < 5; i++) s += (i < rate ? "★" : "☆");
    return s;
}

// 匹配用户id与用户名
QString CommentDialog::matchIDName(std::string id) {
    for (size_t i = 0;i < CommentDialog::m_users.size();i++) {
        if (std::stoi(id) == m_users[i].id) {
            return QString::fromStdString(m_users[i].name);
        }
    }
    return "未知用户";
}
