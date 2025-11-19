// main.cpp
#include "mainwindow.h"
#include "login.h"
#include "dbmanager.h"
#include <QApplication>
#include <QMessageBox.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 初始化数据库（自动创建表）
    if (!DBManager::instance().initDatabase()) {
        QMessageBox::critical(nullptr, "初始化失败", "数据库初始化失败，程序将退出");
        return -1;
    }

    // 显示登录界面
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
