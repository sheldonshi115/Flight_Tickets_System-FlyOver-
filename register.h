#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void on_registerButton_clicked();   // 注册按钮
    void on_cancelButton_clicked();     // 取消按钮

private:
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
