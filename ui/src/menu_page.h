#pragma once

#include <QWidget>
#include <QListWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "FileManager.h"

// 菜单页
// 包含：左侧分类列表、右侧推荐栏 + 菜品卡片滚动区
class MenuPage : public QWidget
{
    Q_OBJECT

public:
    explicit MenuPage(QWidget *parent = nullptr);

    // 加载数据（只需调用一次，或换用户后更新折扣率时不需要重新加载）
    void setData(const QList<Dish_qt> &allItems,
                 const QList<Dish_qt> &bySales,
                 const QList<Dish_qt> &byRating,
                 const QList<Dish_qt> &byComments,
                 const QStringList    &categories);

    // 更新当前用户折扣率并刷新菜品卡片（登录成功后调用）
    void setDiscountRate(double rate);

    // 当前选中的分类文字
    QString currentCategory() const;

signals:
    void addDishClicked(int dishId);      // 点击 + 加入购物车
    void dishCountChanged(int count);     // 当前展示菜品数量变化

private slots:
    void onCategoryChanged(int row);
    void switchRecommendMethod(int index);

private:
    void setupUI();
    void refreshDishList(const QString &category);
    void onSmartRecommend();               // 发起智能推荐请求
    QString buildPrompt();            // 组装固定提示词
    void showAIResponse(const QString &text); // 显示 AI 结果
    void hideAIResponse();                 // 隐藏 AI 区域

    // 数据
    QList<Dish_qt> m_allItems;
    QList<Dish_qt> m_bySales;
    QList<Dish_qt> m_byRating;
    QList<Dish_qt> m_byComments;
    double  m_discountRate = 1.0;
    QString m_recommendMethod = "销量最高";

    // UI
    QListWidget *m_categoryList;
    QFrame *m_recommendBar;
    QPushButton *m_mostSaled;
    QPushButton *m_highScore;
    QPushButton *m_mostCommented;
    QPushButton *m_smartRecommend;       // "智能推荐"按钮
    QScrollArea *m_dishScrollArea;       // 菜品卡片滚动区
    QWidget *m_dishContainer;
    QVBoxLayout *m_dishListLayout;

    // AI 推荐区域
    QWidget *m_aiResponseContainer;      // AI 回复区域容器
    QLabel *m_aiLoadingLabel;            // "正在请求智能推荐..."
    QTextEdit *m_aiResponseText;         // AI 回复纯文本（只读）
    QNetworkAccessManager *m_networkManager; // HTTP 客户端
};
