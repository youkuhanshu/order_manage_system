#include "queue_page.h"

#include <QVBoxLayout>
#include <QLabel>

QueuePage::QueuePage(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background: #F5F5F5;");
    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    auto *label = new QLabel("排队进度 — 待实现", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet(
        "font-size: 20px; color: #CCCCCC; border: none; background: transparent;");
    layout->addWidget(label);
}
