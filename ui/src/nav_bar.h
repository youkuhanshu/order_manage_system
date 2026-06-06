#pragma once

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include "FileManager.h"

/// 顶部导航栏
/// 负责：用户信息显示（头像 + 姓名 + 等级）、三个导航按钮、菜品数量标签
class NavBar : public QFrame
{
    Q_OBJECT

public:
    explicit NavBar(QWidget *parent = nullptr);

    /// 登录成功后刷新用户信息显示
    void setUser(const User &user);

    /// 刷新右侧菜品数量标签
    void setDishCount(int count);

    /// 高亮对应导航按钮（pageIndex: 2=菜单 3=购物车 4=排队）
    void setActiveNav(int pageIndex);

signals:
    void navClicked(int pageIndex);   ///< 点击导航按钮
    void logoutRequested();           ///< 点击"退出登录"

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();

    QFrame      *m_userPanel;
    QLabel      *m_userAvatarLabel;
    QLabel      *m_userNameLabel;
    QLabel      *m_userLevelLabel;
    QPushButton *m_btnMenu;
    QPushButton *m_btnCart;
    QPushButton *m_btnQueue;
    QLabel      *m_dishCountLabel;
};
