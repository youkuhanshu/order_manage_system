#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <vector>
#include "queue_msg.hpp"

/// 美团风格排队进度页
/// 外部接口：setQueueData() 传入当前排队快照；通过 signals 通知 order_system 处理业务
class QueuePage : public QWidget
{
    Q_OBJECT

public:
    explicit QueuePage(QWidget *parent = nullptr);

<<<<<<< HEAD
    /// 刷新排队显示，切换到本页 / 自动叫号 / 取餐后由主窗口调用
=======
    /// 刷新排队显示，切换到本页或点刷新时调用
>>>>>>> 929277828bb293c0d1adb11e36f0a1cd43c54026
    /// currentCall：当前叫到的号
    /// waiting：预约排队队列；taking：取餐排队队列
    /// myQueueId：当前用户自己的取餐号（没有则传 -1）
    void setQueueData(int currentCall,
                      const std::vector<QueueMsg> &waiting,
                      const std::vector<QueueMsg> &taking,
                      int myQueueId);

signals:
<<<<<<< HEAD
    void backToMenuRequested();   ///< 返回菜单
    void pickupRequested(int queueId);  ///< 点「取餐」，传出该号
=======
    void refreshRequested();      ///< 点「刷新」
    void backToMenuRequested();   ///< 返回菜单
    void mealTakenRequested(int queueId); ///< 点「请取餐」，触发评价
>>>>>>> 929277828bb293c0d1adb11e36f0a1cd43c54026

private:
    void setupUI();
    void refreshDisplay();
    QFrame *makeTicketRow(const QueueMsg &msg, int position, bool isMine, bool ready);

    // 数据
    int m_currentCall = 0;
    std::vector<QueueMsg> m_waiting;
    std::vector<QueueMsg> m_taking;
    int m_myQueueId = -1;

    // UI
    QLabel      *m_currentCallLabel;   // 当前叫号大数字
    QLabel      *m_myStatusLabel;      // 我的取餐号状态提示
    QLabel      *m_waitingCountLabel;  // 预约排队人数
    QLabel      *m_takingCountLabel;   // 取餐排队人数
    QVBoxLayout *m_waitingLayout;      // 预约排队列表
    QVBoxLayout *m_takingLayout;       // 取餐排队列表
};
