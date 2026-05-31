#include "order_system.h"

order_system::order_system(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui_order_system)
{
    ui->setupUi(this);
}

order_system::~order_system()
{
    delete ui; 
}