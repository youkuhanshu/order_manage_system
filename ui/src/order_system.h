#pragma once
#include "ui_order_system.h"
#include <QMainWindow>

class order_system : public QMainWindow {
    Q_OBJECT
    
public:
    order_system(QWidget* parent = nullptr);
    ~order_system();

private:
    Ui_order_system* ui;
};