#include "order_system.h"

#include <QApplication>
#pragma comment(lib, "user32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    order_system w;
    w.show();
    return a.exec();
}