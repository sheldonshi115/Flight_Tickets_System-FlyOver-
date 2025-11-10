#include "mainwindow/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;  // 创建主窗口实例
    w.show();
    return a.exec();
}
