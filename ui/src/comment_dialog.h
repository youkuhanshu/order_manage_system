#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QComboBox>
#include <vector>
#include "Comment_msg.hpp"
#include "FileManager.h"       

class CommentService;

class CommentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommentDialog(const Dish_qt                &dish,
                           const std::vector<CommentMsg> &dishComments,
                           const std::vector<User>       &users,
                           CommentService                *commentService,
                           QWidget                       *parent = nullptr);

private slots:
    void onSortChanged(int index);                                                // 响应排序下拉框切换，按时间/评分重排评论列表

private:
    void setupUI(const Dish_qt &dish, const QList<CommentMsg> &comments);        // 构建弹窗整体布局：标题栏、评分概览卡片、评论列表
    void refreshCommentList();                                                    // 清空并重建所有评论卡片（排序或筛选切换后调用）
    void loadComments(const QString &sortType);                                   // 从 CommentService 按排序类型重新拉取评论
    QWidget *makeCommentCard(const CommentMsg &comment, QWidget *parent);         // 创建单条评论卡片：头像、用户名、星级、时间、正文
    QFrame  *makeRatingBar(int star, int count, int total, QWidget *parent);     // 创建星级分布条：N★标签 + 进度条 + 数量
    QString matchIDName(std::string id);                                          // 根据用户ID查找并返回对应用户名

    static QString formatTime(std::time_t t);                                     // 将时间戳格式化为 "MM-dd HH:mm" 字符串
    static QString starStr(int rate);                                             // 将评分数字转为星号字符串 "★★★☆☆"

    // 外部注入数据
    std::vector<User> m_users;
    CommentService   *m_commentService = nullptr;
    std::string       m_dishIdStr;      // 菜品ID字符串，供 CommentService 查询

    // 排序与数据
    QList<CommentMsg> m_comments;     // 当前显示的所有评论
    int m_sortMode = 0;               // 0=按时间, 1=按评分
    QComboBox *m_sortCombo = nullptr;

    // 评论列表容器（排序切换时重建）
    QVBoxLayout *m_listLayout = nullptr;
    QWidget     *m_listContainer = nullptr;
};
