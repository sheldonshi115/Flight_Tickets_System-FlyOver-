#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QScopedPointer> // 新增：用于智能指针管理
#include <QDateTime>      // 新增：用于登录失败锁定
#include "register.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void on_loginButton_clicked();      // 登录按钮
    void on_registerLink_clicked();     // 跳转到注册界面

private:
    Ui::LoginDialog *ui;
    //RegisterDialog *registerDialog;     // 注册界面指针
    QScopedPointer<RegisterDialog> registerDialog; // 优化：智能指针替代裸指针
    int m_failedAttempts = 0; // 新增：失败次数计数
    QDateTime m_lockUntil;    // 新增：锁定截止时间
};

#endif // LOGINDIALOG_H
