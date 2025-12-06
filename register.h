#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();
    void applyTheme(bool isDarkMode);

private:
    void setupUI();
    void applyDarkTheme();
    void applyLightTheme();
    bool validateInput();
    bool registerUser(const QString &account, const QString &password);
    
    Ui::RegisterDialog *ui;
    QLineEdit *m_accountEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmEdit;
    QLineEdit *m_phoneEdit;
    QPushButton *m_registerBtn;
    QPushButton *m_cancelBtn;
    QCheckBox *m_agreeCheckBox;
    QLabel *m_titleLabel;
    bool m_isDarkMode;

private slots:
    void on_registerButton_clicked();
    void on_cancelButton_clicked();

};

#endif // REGISTERDIALOG_H
