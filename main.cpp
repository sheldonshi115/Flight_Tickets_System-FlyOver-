// main.cpp
#include "mainwindow.h"
#include "login.h"
#include "dbmanager.h"
#include "thememanager.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    // 设置 Fusion 风格以确保跨平台一致性
    a.setStyle("Fusion");

    // 初始化深海之光主题（默认浅色模式）
    ThemeManager::instance().setDarkMode(false);

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
