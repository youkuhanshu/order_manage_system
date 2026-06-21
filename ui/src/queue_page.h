#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QTimer>
#include <vector>
#include "queue_msg.hpp"

/// 美团风格排队进度页
/// 外部接口：setQueueData() 传入当前排队快照；通过 signals 通知 order_system 处理业务
class QueuePage : public QWidget
{
    Q_OBJECT

public:
    explicit QueuePage(QWidget *parent = nullptr);                         // 构造排队页面，调用setupUI构建布局

    /// 刷新排队显示（切换到排队页、自动叫号、取餐后由主窗口调用）
    /// currentCall：当前叫到的号
    /// waiting：预约排队队列快照；taking：取餐队列快照
    /// myQueueId：当前用户自己的取餐号（没有则传 -1）
    void setQueueData(int currentCall,
                      const std::vector<QueueMsg> &waiting,
                      const std::vector<QueueMsg> &taking,
                      int myQueueId);

signals:
    void backToMenuRequested();                                            ///< 通知主窗口返回菜单页
    void pickupRequested(int queueId);                                     ///< 用户点击「取餐」按钮，传出取餐号
    void refreshRequested();                                               ///< 用户手动点击刷新按钮，通知主窗口重新拉取队列数据

private:
    void setupUI();                                                        // 构建排队页面布局：顶部叫号横幅 + 左右双列列表 + 底部返回按钮
    void refreshDisplay();                                                 // 根据当前数据刷新叫号数字、状态文字、重建等待/取餐列表
    QFrame *makeTicketRow(const QueueMsg &msg, int position, bool isMine, bool ready);  // 创建单个排队号卡片行

    // 数据
    int m_currentCall = 0;
    std::vector<QueueMsg> m_waiting;
    std::vector<QueueMsg> m_taking;
    int m_myQueueId = -1;

    // UI
    QTimer      *m_refreshTimer;       // 定时刷新排队时间显示
    QLabel      *m_currentCallLabel;   // 当前叫号大数字
    QLabel      *m_myStatusLabel;      // 我的取餐号状态提示
    QLabel      *m_waitingCountLabel;  // 预约排队人数
    QLabel      *m_takingCountLabel;   // 取餐排队人数
    QVBoxLayout *m_waitingLayout;      // 预约排队列表
    QVBoxLayout *m_takingLayout;       // 取餐排队列表
    QPushButton *m_refreshBtn;         // 手动刷新按钮
};
