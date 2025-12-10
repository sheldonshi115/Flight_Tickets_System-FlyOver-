#include "forgotpassworddialog.h"
#include "ui_forgotpassworddialog.h"
#include "dbmanager.h"
#include "utils.h"
#include "emailsender.h"
#include "emailconfig.h"
#include "thememanager.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QDateTime>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

ForgotPasswordDialog::ForgotPasswordDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ForgotPasswordDialog),
      m_codeExpirySeconds(0), m_currentStep(0)
{
    ui->setupUi(this);
    
    setWindowTitle("忘记密码");
    setModal(true);
    setFixedSize(500, 550);
    
    // 应用磨砂玻璃效果背景
    applyModernStyle();
    
    // 初始化邮件发送器
    m_emailSender = new EmailSender(this);
    // 从配置文件读取 QQ 邮箱 SMTP 设置
    m_emailSender->setSMTPConfig(EmailConfig::SMTP_SERVER, 
                                  EmailConfig::SMTP_PORT,
                                  EmailConfig::FROM_EMAIL,
                                  EmailConfig::AUTH_CODE,
                                  EmailConfig::USE_SSL);
    
    // 初始化验证码计时器
    m_verificationTimer = new QTimer(this);
    connect(m_verificationTimer, &QTimer::timeout, this, &ForgotPasswordDialog::onTimerTimeout);
    
    // 连接按钮信号
    connect(ui->btnSendCode, &QPushButton::clicked, this, &ForgotPasswordDialog::onSendCodeClicked);
    connect(ui->btnVerify, &QPushButton::clicked, this, &ForgotPasswordDialog::onVerifyClicked);
    connect(ui->btnChangePassword, &QPushButton::clicked, this, &ForgotPasswordDialog::onChangePasswordClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    // 初始化UI状态
    ui->txtCode->setEnabled(false);
    ui->btnVerify->setEnabled(false);
    ui->txtNewPassword->setEnabled(false);
    ui->btnChangePassword->setEnabled(false);
    ui->lblCodeExpiry->setText("");
    
    // 初始化步骤指示器
    updateStepIndicator(0);
}

void ForgotPasswordDialog::applyModernStyle()
{
    // 获取当前主题
    bool isDark = ThemeManager::instance().isDarkMode();
    
    QString bgColor = isDark ? "rgba(30, 30, 30, 0.95)" : "rgba(255, 255, 255, 0.95)";
    QString textColor = isDark ? "#E0E0E0" : "#333333";
    QString secondaryText = isDark ? "#9E9E9E" : "#666666";
    QString primaryColor = "#42A5F5";
    QString borderColor = isDark ? "rgba(255, 255, 255, 0.1)" : "rgba(0, 0, 0, 0.1)";
    QString inputBg = isDark ? "#2D2D2D" : "#FAFAFA";
    QString inputBorder = isDark ? "#424242" : "#E0E0E0";
    
    // 主窗口样式 - 磨砂玻璃效果
    setStyleSheet(QString(R"(
        QDialog {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 16px;
        }
        QLabel {
            color: %3;
            background: transparent;
        }
        QLineEdit {
            background-color: %4;
            border: 2px solid %5;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 14px;
            color: %3;
        }
        QLineEdit:focus {
            border-color: %6;
        }
        QLineEdit:disabled {
            background-color: %7;
            color: %8;
        }
        QPushButton {
            background-color: %6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1E88E5;
        }
        QPushButton:pressed {
            background-color: #1565C0;
        }
        QPushButton:disabled {
            background-color: %9;
            color: %8;
        }
        QPushButton#btnCancel {
            background-color: transparent;
            color: %8;
            border: 1px solid %5;
        }
        QPushButton#btnCancel:hover {
            background-color: %10;
        }
    )").arg(bgColor, borderColor, textColor, inputBg, inputBorder, 
            primaryColor, isDark ? "#1E1E1E" : "#F5F5F5", secondaryText,
            isDark ? "#424242" : "#BDBDBD", isDark ? "rgba(255,255,255,0.05)" : "rgba(0,0,0,0.05)"));
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setColor(QColor(0, 0, 0, 60));
    shadow->setOffset(0, 8);
    setGraphicsEffect(shadow);
}

void ForgotPasswordDialog::updateStepIndicator(int step)
{
    m_currentStep = step;
    
    // 如果UI中有步骤指示器标签，更新它们
    QString activeColor = "#42A5F5";
    QString inactiveColor = ThemeManager::instance().isDarkMode() ? "#424242" : "#E0E0E0";
    QString completedColor = "#4CAF50";
    
    // 更新步骤标签样式（如果存在）
    QList<QLabel*> stepLabels = {
        findChild<QLabel*>("lblStep1"),
        findChild<QLabel*>("lblStep2"),
        findChild<QLabel*>("lblStep3")
    };
    
    for (int i = 0; i < stepLabels.size(); i++) {
        if (stepLabels[i]) {
            QString color;
            if (i < step) {
                color = completedColor; // 已完成
            } else if (i == step) {
                color = activeColor;    // 当前步骤
            } else {
                color = inactiveColor;  // 未开始
            }
            stepLabels[i]->setStyleSheet(QString("color: %1; font-weight: %2;")
                .arg(color, i == step ? "bold" : "normal"));
        }
    }
}

ForgotPasswordDialog::~ForgotPasswordDialog() {
    if (m_verificationTimer) {
        m_verificationTimer->stop();
    }
    delete ui;
}

QString ForgotPasswordDialog::generateVerificationCode() {
    // 生成6位随机数字验证码
    QString code;
    for (int i = 0; i < 6; ++i) {
        code += QString::number(QRandomGenerator::global()->bounded(10));
    }
    return code;
}

bool ForgotPasswordDialog::sendVerificationEmail(const QString &email, const QString &code) {
    // 验证邮箱格式
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "错误", "邮箱格式不正确");
        return false;
    }
    
    // 构建邮件主题和内容
    QString subject = "飞越订票系统 - 密码重置验证码";
    QString body = "尊敬的用户，\n\n";
    body += "您的验证码为：" + code + "\n\n";
    body += "请在 5 分钟内输入此验证码完成密码重置。\n";
    body += "如果您未申请重置密码，请忽略此邮件。\n\n";
    body += "此邮件由系统自动发送，请勿回复。\n\n";
    body += "飞越订票系统团队";
    
    qDebug() << "使用 SMTP 发送验证码邮件到:" << email;
    
    // 使用 EmailSender 发送邮件
    bool result = m_emailSender->sendEmail(email, subject, body);
    
    if (result) {
        qDebug() << "邮件发送请求已提交";
        return true;
    } else {
        QMessageBox::critical(this, "错误", "邮件发送失败，请检查网络连接");
        return false;
    }
}

void ForgotPasswordDialog::onSendCodeClicked() {
    m_userEmail = ui->txtEmail->text().trimmed();
    
    if (m_userEmail.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入邮箱地址");
        return;
    }
    
    // 检查邮箱是否在数据库中存在
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库连接失败");
        return;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT account FROM users WHERE email = :email");
    query.bindValue(":email", m_userEmail);
    
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }
    
    if (!query.next()) {
        QMessageBox::warning(this, "提示", "该邮箱未在系统中注册");
        return;
    }
    
    // 生成验证码
    m_generatedCode = generateVerificationCode();
    
    // 发送邮件
    if (sendVerificationEmail(m_userEmail, m_generatedCode)) {
        QMessageBox::information(this, "成功", "验证码已发送到您的邮箱，请在5分钟内输入");
        
        // 启动计时器（5分钟 = 300秒）
        m_codeExpirySeconds = 300;
        m_verificationTimer->start(1000); // 每秒更新一次
        
        // 禁用邮箱和发送按钮
        ui->txtEmail->setEnabled(false);
        ui->btnSendCode->setEnabled(false);
        
        // 启用验证码输入和验证按钮
        ui->txtCode->setEnabled(true);
        ui->btnVerify->setEnabled(true);
        
        // 更新步骤指示器到第2步
        updateStepIndicator(1);
        
        updateCodeExpiryLabel();
        
        qDebug() << "验证码:" << m_generatedCode << "（仅用于测试，实际应发送到邮箱）";
    } else {
        QMessageBox::critical(this, "错误", "邮件发送失败，请稍后重试");
    }
}

void ForgotPasswordDialog::onVerifyClicked() {
    QString inputCode = ui->txtCode->text().trimmed();
    
    if (inputCode.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入验证码");
        return;
    }
    
    if (inputCode != m_generatedCode) {
        QMessageBox::warning(this, "错误", "验证码错误");
        return;
    }
    
    // 验证成功
    m_verificationTimer->stop();
    
    QMessageBox::information(this, "成功", "验证成功，请输入新密码");
    
    // 禁用验证码输入和验证按钮
    ui->txtCode->setEnabled(false);
    ui->btnVerify->setEnabled(false);
    ui->lblCodeExpiry->setText("✓ 验证成功");
    ui->lblCodeExpiry->setStyleSheet("color: #4CAF50; font-weight: bold;");
    
    // 启用新密码输入和修改按钮
    ui->txtNewPassword->setEnabled(true);
    ui->btnChangePassword->setEnabled(true);
    
    // 更新步骤指示器到第3步
    updateStepIndicator(2);
}

void ForgotPasswordDialog::onChangePasswordClicked() {
    QString newPassword = ui->txtNewPassword->text();
    
    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入新密码");
        return;
    }
    
    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "提示", "密码至少6位");
        return;
    }
    
    // 在数据库中更新密码
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库连接失败");
        return;
    }
    
    // 生成新的盐值和加密密码
    QString salt = generateSalt();
    QString encryptedPwd = hashPassword(newPassword, salt);
    
    QSqlQuery query(db);
    query.prepare("UPDATE users SET password = :password, salt = :salt WHERE email = :email");
    query.bindValue(":password", encryptedPwd);
    query.bindValue(":salt", salt);
    query.bindValue(":email", m_userEmail);
    
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "更新密码失败：" + query.lastError().text());
        return;
    }
    
    QMessageBox::information(this, "成功", "密码修改成功，请用新密码登录");
    this->accept();
}

void ForgotPasswordDialog::onTimerTimeout() {
    m_codeExpirySeconds--;
    updateCodeExpiryLabel();
    
    if (m_codeExpirySeconds <= 0) {
        m_verificationTimer->stop();
        QMessageBox::warning(this, "提示", "验证码已过期，请重新申请");
        resetUI();
    }
}

void ForgotPasswordDialog::updateCodeExpiryLabel() {
    int minutes = m_codeExpirySeconds / 60;
    int seconds = m_codeExpirySeconds % 60;
    ui->lblCodeExpiry->setText(QString("验证码有效期: %1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
}

void ForgotPasswordDialog::resetUI() {
    ui->txtEmail->setEnabled(true);
    ui->txtEmail->clear();
    ui->btnSendCode->setEnabled(true);
    
    ui->txtCode->setEnabled(false);
    ui->txtCode->clear();
    ui->btnVerify->setEnabled(false);
    
    ui->txtNewPassword->setEnabled(false);
    ui->txtNewPassword->clear();
    ui->btnChangePassword->setEnabled(false);
    
    ui->lblCodeExpiry->setText("");
    m_userEmail.clear();
    m_generatedCode.clear();
}
