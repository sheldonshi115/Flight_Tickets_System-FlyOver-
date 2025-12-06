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

    // 关键1：初始登录窗口改为堆对象（模态显示，保留原有体验）
    LoginDialog *loginDialog = new LoginDialog();
    QString acc = loginDialog->get_account();
    loginDialog->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动释放内存
    // 初始登录用exec()（模态），登录成功后再创建主窗口
    if (loginDialog->exec() != QDialog::Accepted) {
        delete loginDialog; // 未登录成功，手动释放
        return 0;
    }

    // 关键2：主窗口改为堆对象（避免栈对象生命周期问题）
    MainWindow *mainWindow = new MainWindow();
    mainWindow->set_account(acc);
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->show();

    return a.exec();
}
