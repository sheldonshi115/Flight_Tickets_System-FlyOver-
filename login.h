#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
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
    void applyTheme(bool isDarkMode);

private slots:
    void on_loginButton_clicked();
    void on_registerLink_clicked();
    void onRememberMeToggled(bool checked);

private:
    void setupUI();
    void applyDarkTheme();
    void applyLightTheme();

    Ui::LoginDialog *ui;
    RegisterDialog *registerDialog;
    QLineEdit *m_accountEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginBtn;
    QPushButton *m_registerLink;
    QCheckBox *m_rememberMe;
    QLabel *m_titleLabel;
    bool m_isDarkMode;
};

#endif // LOGINDIALOG_H
