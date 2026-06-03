#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include "FileManager.h"

class CommentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommentDialog(const Dish_qt &dish, QWidget *parent);

private:
    void setupUI(const Dish_qt &dish, const QList<CommentMsg> &comments);
    QWidget *makeCommentCard(const CommentMsg &comment, QWidget *parent);
    QFrame  *makeRatingBar(int star, int count, int total, QWidget *parent);
    QString matchIDName(std::string id);

    static QString formatTime(std::time_t t);
    static QString starStr(int rate);
    static std::vector<User> m_users;
};
