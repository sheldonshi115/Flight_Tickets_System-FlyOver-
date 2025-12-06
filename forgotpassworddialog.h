#ifndef FORGOTPASSWORDDIALOG_H
#define FORGOTPASSWORDDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QMap>

namespace Ui {
class ForgotPasswordDialog;
}

class EmailSender;

class ForgotPasswordDialog : public QDialog {
    Q_OBJECT

public:
    explicit ForgotPasswordDialog(QWidget *parent = nullptr);
    ~ForgotPasswordDialog();

private slots:
    void onSendCodeClicked();        // 发送验证码
    void onVerifyClicked();          // 验证验证码
    void onChangePasswordClicked();  // 更改密码
    void onTimerTimeout();           // 验证码倒计时超时

private:
    Ui::ForgotPasswordDialog *ui;
    EmailSender *m_emailSender;      // 邮件发送器
    
    // 验证码相关
    QString m_generatedCode;             // 存储生成的验证码
    QString m_userEmail;                 // 存储用户输入的邮箱
    int m_codeExpirySeconds;            // 验证码过期倒计时
    QTimer *m_verificationTimer;        // 验证码倒计时计时器
    
    // 邮件发送相关
    bool sendVerificationEmail(const QString &email, const QString &code);
    QString generateVerificationCode();  // 生成随机验证码
    void updateCodeExpiryLabel();       // 更新过期时间显示
    void resetUI();                     // 重置UI
};

#endif // FORGOTPASSWORDDIALOG_H
