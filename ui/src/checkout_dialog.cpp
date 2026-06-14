#include "checkout_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <ctime>

CheckoutDialog::CheckoutDialog(const QStringList &dishNames, const QList<int> &dishIds, double total, int queueId, int userId, QWidget *parent)
    : QDialog(parent)
    , m_dishNames(dishNames)
    , m_dishIds(dishIds)
    , m_total(total)
    , m_queueId(queueId)
    , m_userId(userId)
{
    setupUI();
}

void CheckoutDialog::setupUI()
{
    setWindowTitle("取餐成功");
    setMinimumWidth(440);
    setMaximumWidth(520);
    setStyleSheet("QDialog { background: #F5F5F5; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ================================================================
    // 顶部标题栏：结算金额 + 取餐号
    // ================================================================
    auto *headerCard = new QFrame(this);
    headerCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-bottom: 1px solid #F0F0F0; }");
    auto *headerLayout = new QVBoxLayout(headerCard);
    headerLayout->setContentsMargins(24, 20, 24, 20);
    headerLayout->setSpacing(10);

    // 结算成功标题
    auto *titleLabel = new QLabel("🎉 取餐成功", headerCard);
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    headerLayout->addWidget(titleLabel);

    // 金额 + 取餐号
    auto *infoRow = new QHBoxLayout();
    infoRow->setSpacing(24);

    auto *totalLabel = new QLabel(
        QString("实付 <span style='color:#FF4D2E;font-size:22px;font-weight:bold;'>¥%1</span>")
            .arg(m_total, 0, 'f', 2),
        headerCard);
    totalLabel->setStyleSheet(
        "font-size: 14px; color: #666666; border: none; background: transparent;");
    infoRow->addWidget(totalLabel);

    auto *queueLabel = new QLabel(
        QString("取餐号 <span style='color:#0085FF;font-size:18px;font-weight:bold;'>%1</span>")
            .arg(m_queueId),
        headerCard);
    queueLabel->setStyleSheet(
        "font-size: 14px; color: #666666; border: none; background: transparent;");
    infoRow->addWidget(queueLabel);
    infoRow->addStretch();

    headerLayout->addLayout(infoRow);
    mainLayout->addWidget(headerCard);

    // ================================================================
    // 中间内容区（可滚动）
    // ================================================================
    auto *bodyWidget = new QWidget(this);
    bodyWidget->setStyleSheet("background: #FFFFFF;");
    auto *bodyLayout = new QVBoxLayout(bodyWidget);
    bodyLayout->setContentsMargins(24, 20, 24, 20);
    bodyLayout->setSpacing(16);

    // ---- 本次菜品 ----
    auto *dishSectionLabel = new QLabel("本次消费菜品", bodyWidget);
    dishSectionLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    bodyLayout->addWidget(dishSectionLabel);

    // 菜品标签行
    auto *dishTagsLayout = new QHBoxLayout();
    dishTagsLayout->setSpacing(8);
    for (const auto &name : m_dishNames) {
        auto *tag = new QLabel(name, bodyWidget);
        tag->setStyleSheet(
            "font-size: 12px; color: #555555; background: #F5F7FA;"
            "border: 1px solid #E8ECF0; border-radius: 14px;"
            "padding: 4px 12px;");
        dishTagsLayout->addWidget(tag);
    }
    dishTagsLayout->addStretch();
    bodyLayout->addLayout(dishTagsLayout);

    // 分隔线
    auto *divider1 = new QFrame(bodyWidget);
    divider1->setFrameShape(QFrame::HLine);
    divider1->setStyleSheet("QFrame { background: #F0F0F0; border: none; max-height: 1px; }");
    bodyLayout->addWidget(divider1);

    // ---- 评分 ----
    auto *ratingLabel = new QLabel("评价菜品", bodyWidget);
    ratingLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    bodyLayout->addWidget(ratingLabel);

    auto *starsRow = new QHBoxLayout();
    starsRow->setSpacing(6);
    starsRow->setAlignment(Qt::AlignLeft);

    const QString starUnchecked = R"(
        QPushButton {
            background: transparent; border: none;
            font-size: 32px; color: #DDDDDD;
        }
        QPushButton:hover { color: #FFB800; }
    )";
    const QString starChecked = R"(
        QPushButton {
            background: transparent; border: none;
            font-size: 32px; color: #FFB800;
        }
    )";

    for (int i = 0; i < 5; i++) {
        m_starBtns[i] = new QPushButton("★", bodyWidget);
        m_starBtns[i]->setFixedSize(44, 44);
        m_starBtns[i]->setCursor(Qt::PointingHandCursor);
        m_starBtns[i]->setStyleSheet(starUnchecked);
        const int star = i + 1;
        connect(m_starBtns[i], &QPushButton::clicked, this, [this, star]() {
            onStarClicked(star);
        });
        starsRow->addWidget(m_starBtns[i]);
    }
    starsRow->addStretch();
    bodyLayout->addLayout(starsRow);

    // ---- 文字评价 ----
    auto *commentLabel = new QLabel("写点评价（选填）", bodyWidget);
    commentLabel->setStyleSheet(
        "font-size: 14px; font-weight: bold; color: #333333;"
        "border: none; background: transparent;");
    bodyLayout->addWidget(commentLabel);

    m_commentEdit = new QTextEdit(bodyWidget);
    m_commentEdit->setPlaceholderText("口味、服务、环境……分享你的用餐体验");
    m_commentEdit->setFixedHeight(80);
    m_commentEdit->setStyleSheet(R"(
        QTextEdit {
            background: #F9FAFB;
            border: 1px solid #E8ECF0;
            border-radius: 8px;
            font-size: 13px; color: #333333;
            padding: 10px;
        }
        QTextEdit:focus { border-color: #0085FF; background: #FFFFFF; }
    )");
    bodyLayout->addWidget(m_commentEdit);

    bodyLayout->addStretch();

    // 如果内容多就加滚动
    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(bodyWidget);
    scrollArea->setStyleSheet(
        "QScrollArea { background: #FFFFFF; border: none; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { background: #DDDDDD; border-radius: 2px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }");
    mainLayout->addWidget(scrollArea, 1);

    // ================================================================
    // 底部按钮栏
    // ================================================================
    auto *footerCard = new QFrame(this);
    footerCard->setStyleSheet(
        "QFrame { background: #FFFFFF; border-top: 1px solid #F0F0F0; }");
    auto *footerLayout = new QHBoxLayout(footerCard);
    footerLayout->setContentsMargins(24, 12, 24, 12);
    footerLayout->setSpacing(12);

    auto *skipBtn = new QPushButton("跳过评价", footerCard);
    skipBtn->setFixedHeight(40);
    skipBtn->setCursor(Qt::PointingHandCursor);
    skipBtn->setStyleSheet(R"(
        QPushButton {
            background: #F5F5F5; color: #999999; border: none;
            border-radius: 8px; font-size: 14px; padding: 0 24px;
        }
        QPushButton:hover { background: #E8E8E8; color: #666666; }
    )");
    connect(skipBtn, &QPushButton::clicked, this, &QDialog::reject);
    footerLayout->addWidget(skipBtn);

    footerLayout->addStretch();

    m_submitBtn = new QPushButton("提交评价", footerCard);
    m_submitBtn->setFixedHeight(40);
    m_submitBtn->setCursor(Qt::PointingHandCursor);
    m_submitBtn->setEnabled(false);
    m_submitBtn->setStyleSheet(R"(
        QPushButton {
            background: #0085FF; color: #FFFFFF; border: none;
            border-radius: 8px; font-size: 14px; font-weight: bold;
            padding: 0 28px;
        }
        QPushButton:hover   { background: #0073E6; }
        QPushButton:pressed { background: #0060BF; }
        QPushButton:disabled {
            background: #CCE0FF; color: #FFFFFF;
        }
    )");
    connect(m_submitBtn, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addWidget(m_submitBtn);

    mainLayout->addWidget(footerCard);

    resize(480, 420);
}

void CheckoutDialog::onStarClicked(int star)
{
    // 再次点击同一颗星 → 取消评分；否则设为新评分
    if (m_rating == star) {
        m_rating = 0;
    } 
    else {
        m_rating = star;
    }
    updateStars();
    m_submitBtn->setEnabled(m_rating > 0);
}

void CheckoutDialog::updateStars()
{
    for (int i = 0; i < 5; i++) {
        if (i < m_rating) {
            m_starBtns[i]->setStyleSheet(R"(
                QPushButton {
                    background: transparent; border: none;
                    font-size: 32px; color: #FFB800;
                }
                QPushButton:hover { color: #E6A500; }
            )");
        } 
        else {
            m_starBtns[i]->setStyleSheet(R"(
                QPushButton {
                    background: transparent; border: none;
                    font-size: 32px; color: #DDDDDD;
                }
                QPushButton:hover { color: #FFB800; }
            )");
        }
    }
}

CommentMsg CheckoutDialog::getComment() const
{
    std::vector<std::string> dishIdStrs;
    for (int id : m_dishIds) {
        dishIdStrs.push_back(std::to_string(id));
    }
    return CommentMsg(
        std::to_string(m_userId),
        dishIdStrs,
        m_commentEdit->toPlainText().toStdString(),
        m_rating,
        std::time(nullptr)
    );
}
