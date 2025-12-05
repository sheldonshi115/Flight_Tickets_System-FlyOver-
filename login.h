#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "register.h" // 包含注册界面头文件

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
    RegisterDialog *registerDialog;     // 注册界面指针
};

#endif // LOGINDIALOG_H
