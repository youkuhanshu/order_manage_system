#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QComboBox>
#include "FileManager.h"

class CommentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommentDialog(const Dish_qt &dish, QWidget *parent);

private slots:
    void onSortChanged(int index);

private:
    void setupUI(const Dish_qt &dish, const QList<CommentMsg> &comments);
    void refreshCommentList();
    QWidget *makeCommentCard(const CommentMsg &comment, QWidget *parent);
    QFrame  *makeRatingBar(int star, int count, int total, QWidget *parent);
    QString matchIDName(std::string id);

    static QString formatTime(std::time_t t);
    static QString starStr(int rate);
    static std::vector<User> m_users;

    // 排序与数据
    QList<CommentMsg> m_comments;     // 当前显示的所有评论
    int m_sortMode = 0;               // 0=按时间, 1=按评分
    QComboBox *m_sortCombo = nullptr;

    // 评论列表容器（排序切换时重建）
    QVBoxLayout *m_listLayout = nullptr;
    QWidget     *m_listContainer = nullptr;
};
