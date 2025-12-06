// main.cpp
#include "mainwindow.h"
#include "login.h"
#include "dbmanager.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 加载全局样式表
    QFile styleFile(":/resources/style.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        QString style = styleFile.readAll();
        a.setStyleSheet(style);
        styleFile.close();
    }

    // 初始化数据库（自动创建表）
    if (!DBManager::instance().initDatabase()) {
        QMessageBox::critical(nullptr, "初始化失败", "数据库初始化失败，程序将退出");
        return -1;
    }

    // 显示登录界面
    LoginDialog login;
    login.applyTheme(false);  // 默认浅色主题
    if (login.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
