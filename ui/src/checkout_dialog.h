#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QStringList>
#include <QList>
#include "FileManager.h"

/// 结算后评论弹窗
/// 显示结算金额、取餐号，允许用户对本次所有菜品打一个总分 + 写一段评价
class CheckoutDialog : public QDialog
{
    Q_OBJECT

public:
    /// @param dishNames  本次结算的菜品名称（去重，用于展示）
    /// @param dishIds    对应的菜品 ID（用于构建 CommentMsg）
    /// @param total      实付金额
    /// @param queueId    取餐号
    /// @param userId     当前用户 ID
    /// @param parent
    explicit CheckoutDialog(const QStringList &dishNames,
                            const QList<int>   &dishIds,
                            double              total,
                            int                 queueId,
                            int                 userId,
                            QWidget            *parent = nullptr);

    /// 获取用户提交的评论（rating == 0 表示未评分，调用者应跳过）
    CommentMsg getComment() const;

private slots:
    void onStarClicked(int star);

private:
    void setupUI();
    void updateStars();

    // 数据
    QStringList m_dishNames;
    QList<int>  m_dishIds;
    double      m_total;
    int         m_queueId;
    int         m_userId;
    int         m_rating = 0;       // 0 = 未评分

    // UI
    QPushButton *m_starBtns[5];
    QTextEdit   *m_commentEdit;
    QPushButton *m_submitBtn;
};
