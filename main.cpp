// main.cpp
#include "mainwindow.h"
#include "login.h"
#include "dbmanager.h"
#include "thememanager.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox.h>
#include <QProcess>

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
    LoginDialog loginDialog;
    loginDialog.show();

    int exitCode = a.exec();
    
    // 检查退出码，如果是 1000 表示需要重启
    if (exitCode == 1000) {
        qDebug() << "[main] 检测到重启信号，准备重启应用程序";
        
        // 使用 QProcess 启动新实例
        QString program = QApplication::applicationFilePath();
        QStringList arguments = QApplication::arguments();
        arguments.removeFirst(); // 移除程序路径
        
        // 分离进程启动（不等待子进程结束）
        bool started = QProcess::startDetached(program, arguments);
        
        if (started) {
            qDebug() << "[main] 新实例已启动";
        } else {
            qWarning() << "[main] 无法启动新实例";
        }
    }
    
    return exitCode;
}
