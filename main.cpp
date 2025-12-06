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

    // 显示登录窗口
    // 登录成功后，login.cpp 中会自动创建 MainWindow 并调用 setUserProfile
    // 登录失败则程序退出
    LoginDialog *loginDialog = new LoginDialog();
    loginDialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
    loginDialog->show(); // 使用 show() 而不是 exec()，让应用继续运行

    return a.exec();
}
