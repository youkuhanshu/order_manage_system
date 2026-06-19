#include "menu_page.h"
#include "dish_card.h"
#include "comment_dialog.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QStyle>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

// ---- DeepSeek API 配置 ----
const QString DEEPSEEK_API_KEY = "sk-3f77a42174714886b17756b869fee34a";
const QString DEEPSEEK_API_URL = "https://api.deepseek.com/v1/chat/completions";

MenuPage::MenuPage(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void MenuPage::setupUI()
{
    auto *pageLayout = new QHBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    // 左侧分类列表
    m_categoryList = new QListWidget(this); // 可点击的列表控件
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
            this, &MenuPage::onCategoryChanged); // onCategoryChanged接收currentRowChanged传递的row参数，刷新菜单

    pageLayout->addWidget(m_categoryList);

    // 右侧容器（推荐栏 + 滚动区）
    auto *rightContainer = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    // 推荐方式切换栏：单独作为一个整体，便于显示/隐藏
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

    m_smartRecommend = new QPushButton("智能推荐", m_recommendBar);
    m_smartRecommend->setStyleSheet(recBtnStyle);
    m_smartRecommend->setCursor(Qt::PointingHandCursor);

    barLayout->addWidget(m_mostSaled);
    barLayout->addWidget(m_highScore);
    barLayout->addWidget(m_mostCommented);
    barLayout->addWidget(m_smartRecommend);
    barLayout->addStretch();

    connect(m_mostSaled, &QPushButton::clicked, this, [this]() { switchRecommendMethod(0); });
    connect(m_highScore, &QPushButton::clicked, this, [this]() { switchRecommendMethod(1); });
    connect(m_mostCommented, &QPushButton::clicked, this, [this]() { switchRecommendMethod(2); });
    connect(m_smartRecommend, &QPushButton::clicked, this, [this]() { switchRecommendMethod(3); });

    rightLayout->addWidget(m_recommendBar);

    // AI 推荐回复区域（默认隐藏）
    m_aiResponseContainer = new QWidget(rightContainer);
    m_aiResponseContainer->setVisible(false);
    auto *aiLayout = new QVBoxLayout(m_aiResponseContainer);
    aiLayout->setContentsMargins(16, 12, 16, 12);
    aiLayout->setSpacing(8);

    m_aiLoadingLabel = new QLabel("正在请求 DeepSeek 智能推荐...", m_aiResponseContainer);
    m_aiLoadingLabel->setAlignment(Qt::AlignCenter);
    m_aiLoadingLabel->setStyleSheet(
        "font-size: 14px; color: #0085FF; padding: 40px;"
        "border: none; background: transparent;");
    m_aiLoadingLabel->setVisible(false); // 默认隐藏
    aiLayout->addWidget(m_aiLoadingLabel);

    m_aiResponseText = new QTextEdit(m_aiResponseContainer);
    m_aiResponseText->setReadOnly(true);
    m_aiResponseText->setStyleSheet(R"(
        QTextEdit {
            background: #FFFFFF; border: 1px solid #E8E8E8; border-radius: 10px;
            font-size: 14px; color: #333333; padding: 12px;
        }
    )");
    m_aiResponseText->setVisible(false);
    aiLayout->addWidget(m_aiResponseText, 1);

    m_networkManager = new QNetworkAccessManager(this);

    rightLayout->addWidget(m_aiResponseContainer, 1);

    // 菜品滚动区
    m_dishScrollArea = new QScrollArea(rightContainer); // 带有滚动条的容器
    m_dishScrollArea->setWidgetResizable(true);
    m_dishScrollArea->setStyleSheet(R"(
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
    m_dishListLayout = new QVBoxLayout(m_dishContainer); // 此时m_dishContainer为空，后续添加卡片
    m_dishListLayout->setContentsMargins(16, 12, 16, 12);
    m_dishListLayout->setSpacing(10);
    m_dishListLayout->addStretch();

    m_dishScrollArea->setWidget(m_dishContainer);
    rightLayout->addWidget(m_dishScrollArea, 1);

    pageLayout->addWidget(rightContainer, 1);
}

// 全部菜品，销量排序列表，评分排序列表，评论数排序列表，类型列表
void MenuPage::setData(const QList<Dish_qt> &allItems,
                       const QList<Dish_qt> &bySales,
                       const QList<Dish_qt> &byRating,
                       const QList<Dish_qt> &byComments,
                       const QStringList    &categories)
{
    m_allItems = allItems;
    m_bySales = bySales;
    m_byRating = byRating;
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
    hideAIResponse(); // 隐藏AI区域，否则菜单显示不出来
    refreshDishList(m_categoryList->item(row)->text());
}

void MenuPage::switchRecommendMethod(int index)
{
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
        case 3:
            m_recommendMethod = "智能推荐";
            break;
        default:
            break;
    }

    m_mostSaled->setProperty("active", index == 0);
    m_highScore->setProperty("active", index == 1);
    m_mostCommented->setProperty("active", index == 2);
    m_smartRecommend->setProperty("active", index == 3);

    for (auto *btn : {m_mostSaled, m_highScore, m_mostCommented, m_smartRecommend}) {
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

    // 默认：显示菜品卡片区、隐藏 AI 推荐区（只有"智能推荐"分支会反过来）
    // 修复 bug：之前从"智能推荐"切到普通分类/全部时，没把卡片区重新显示，导致菜单一片空白——把恢复显示提到这里，覆盖所有非智能推荐路径。
    m_dishScrollArea->setVisible(true);

    int count = 0;

    auto makeCard = [&](const Dish_qt &dish) {
        // 查找该菜品在销量 Top5 / 好评 Top5 中的名次（找不到则为 0）
        int salesRank = 0, ratingRank = 0;
        for (int i = 0; i < m_bySales.size(); i++) {
            if (m_bySales[i].id == dish.id) { 
                salesRank = i + 1; 
                break; 
            }
        }
        for (int i = 0; i < m_byRating.size(); i++) {
            if (m_byRating[i].id == dish.id) { 
                ratingRank = i + 1; 
                break; 
            }
        }
        auto *card = new DishCard(dish, m_discountRate, salesRank, ratingRank, m_dishContainer);
        connect(card, &DishCard::addClicked, this, [this](int id) { emit addDishClicked(id); });
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
        QList<Dish_qt> recommend_dishes;
        if (m_recommendMethod == "销量最高") {
            recommend_dishes = m_bySales;
        }
        else if (m_recommendMethod == "评分最高") {
            recommend_dishes = m_byRating;
        }
        else if (m_recommendMethod == "最多评价") {
            recommend_dishes = m_byComments;
        }
        else if (m_recommendMethod == "智能推荐") {
            m_dishScrollArea->setVisible(false); // 隐藏菜品卡片
            onSmartRecommend();
            return;  // 不执行后续 makeCard / emit
        }
        m_dishScrollArea->setVisible(true);
        hideAIResponse();
        for (const auto &item : recommend_dishes) makeCard(item);
    }
    else {
        for (const auto &item : m_allItems) {
            if (item.category == category || category == "全部")
                makeCard(item);
        }
    }

    m_dishListLayout->addStretch();
    emit dishCountChanged(count);
}

//  发起智能推荐请求
void MenuPage::onSmartRecommend()
{
    // 显示 loading
    m_aiResponseContainer->setVisible(true);
    m_aiLoadingLabel->setVisible(true);
    m_aiResponseText->setVisible(false);

    // 官方请求示例：
    /*
       curl https://api.deepseek.com/chat/completions \
        -H "Content-Type: application/json" \
        -H "Authorization: Bearer ${DEEPSEEK_API_KEY}" \
        -d '{
                "model": "deepseek-v4-pro",
                "messages": [
                    {"role": "system", "content": "You are a helpful assistant."},
                    {"role": "user", "content": "Hello!"}
                ],
                "thinking": {"type": "enabled"},
                "reasoning_effort": "high",
                "stream": false
            }'
    */

    // 构造请求
    QNetworkRequest request{QUrl(DEEPSEEK_API_URL)}; // deepseek base_url
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + DEEPSEEK_API_KEY).toUtf8());

    // 请求头
    QJsonObject body; 
    body["model"] = "deepseek-v4-flash"; // 模型

    QJsonObject think;
    think["type"] = "disabled"; // 非思考模式
    body["thinking"] = think;

    body["reasoning_effort"] = "low"; // 不需要思考很多
    body["stream"] = false; // 单次对话
    body["max_tokens"] = 1024; // 最多回复1024个token，太多了显示不下

    // 具体发送的文字内容
    QJsonArray messages;

    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "你是一个专业的美食推荐助手。你会根据当前时间、季节和天气因素，从菜单中挑选最适合的菜品推荐给顾客。回复简洁亲切";
    messages.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = buildPrompt();
    messages.append(userMsg);

    body["messages"] = messages;

    // POST 请求：把 JSON 正文贴到 DeepSeek 服务器
    // request  = 信封（URL + 请求头 + API Key）
    // QJsonDocument(body).toJson() = 把 body 压缩成 JSON 字节串作为正文
    // reply = 回执单，此时请求已发出，但回复还没到
    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(body).toJson());

    // 绑定回调：当服务器回复到达（或超时/断网）时，自动执行 {} 里的代码
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {

        reply->deleteLater(); // 标记 reply 待删除，等事件循环跑完这一轮再安全释放内存

        // 检查网络错误（断网、服务器挂了、API Key 无效等）
        if (reply->error() != QNetworkReply::NoError) {
            showAIResponse(QString("网络请求失败：%1").arg(reply->errorString()));
            return;
        }

        // 读取服务器返回的原始字节，解析为 JSON 文档
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        // 从 JSON 中取出 choices 数组（OpenAI-api规范格式）
        QJsonArray choices = doc.object()["choices"].toArray();
        if (choices.isEmpty()) {
            showAIResponse("DeepSeek 未返回任何推荐内容。");
            return;
        }

        // 层层剥取：choices[0] → message → content（即推荐文字）
        QString content = choices[0].toObject()["message"].toObject()["content"].toString();
        if (content.isEmpty()) {
            showAIResponse("DeepSeek 返回内容为空，请稍后重试。");
            return;
        }

        // 把推荐文字显示到界面上
        showAIResponse(content);
    });
}

//  组装固定提示词（菜单数据 + 时间 + 地点）
QString MenuPage::buildPrompt()
{
    QDateTime now = QDateTime::currentDateTime();
    QString timeInfo = QString("地点：长沙\n当前时间：%1\n星期：%2")
        .arg(now.toString("yyyy年M月d日 HH:mm"))
        .arg(now.toString("dddd"));

    QString menu;
    for (const auto &dish : m_allItems) {
        menu += QString("%1 %2 %3 %4 %5\n")
            .arg(dish.name)
            .arg(dish.price)
            .arg(dish.description)
            .arg(dish.rating)
            .arg(dish.category);
    }

    return QString(
        "请根据以下信息为顾客推荐3-5道最适合当前享用的菜品：\n\n"
        "%1\n\n完整菜单：\n%3\n\n"
        "要求：\n"
        "1. 菜单格式为：菜名 价格 描述 销量 评分 类型\n"
        "2. 综合考虑当前时间、季节以及长沙当地的天气因素\n"
        "3. 推荐3-5道菜品，每道菜品单独一行\n"
        "4. 格式：菜名 - 推荐理由（只要一句话）")
        .arg(timeInfo)
        .arg(menu);
}

//  显示 AI 推荐结果
void MenuPage::showAIResponse(const QString &text)
{
    m_aiLoadingLabel->setVisible(false);
    m_aiResponseText->setPlainText(text);
    m_aiResponseText->setVisible(true);
}

//  隐藏 AI 区域
void MenuPage::hideAIResponse()
{
    m_aiResponseContainer->setVisible(false);
    m_aiLoadingLabel->setVisible(false);
    m_aiResponseText->setVisible(false);
}
