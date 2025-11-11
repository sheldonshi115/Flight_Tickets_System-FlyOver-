#ifndef DIALOG_LOGIN_H
#define DIALOG_LOGIN_H

#include <QDialog>

namespace Ui {
class Dialog_login;
}

class Dialog_login : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_login(QWidget *parent = nullptr);
    ~Dialog_login();
    QString getAccount() const;
    QString getPwd() const;
private slots:
    void on_login_button_clicked();
    void on_login_rejected_clicked();
private:
    Ui::Dialog_login *ui;
};

#endif // DIALOG_LOGIN_H
