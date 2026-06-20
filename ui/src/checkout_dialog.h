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
    explicit CheckoutDialog(const QStringList &dishNames,           // 构造结算评价弹窗：传入菜品名列表、菜品ID列表、金额、取餐号、用户ID
                            const QList<int>   &dishIds,
                            double              total,
                            int                 queueId,
                            int                 userId,
                            QWidget            *parent = nullptr);

    /// 获取用户提交的评论（rating == 0 表示未评分，调用者应跳过）
    CommentMsg getComment() const;                                  // 构造并返回CommentMsg对象（含用户ID、菜品ID列表、评论内容、评分、当前时间戳）

private slots:
    void onStarClicked(int star);                                   // 点击星级按钮：设置/取消评分（再次点击同一星可取消），更新提交按钮状态

private:
    void setupUI();                                                 // 构建弹窗整体布局：顶部金额卡片、中间菜品标签+星级+评论输入、底部按钮
    void updateStars();                                             // 根据当前m_rating刷新五颗星按钮的亮/暗样式

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
