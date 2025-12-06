#include "register.h"
#include "ui_register.h"
#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QApplication>
#include <QRegularExpression>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(nullptr),
    m_isDarkMode(false)
{
    setWindowTitle("FlyOver - 注册");
    setFixedSize(500, 700);
    setWindowIcon(QIcon(":/images/logo.png"));
    setupUI();
    applyLightTheme();
}

RegisterDialog::~RegisterDialog()
{
    if (ui) delete ui;
}

void RegisterDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(16);
    setStyleSheet("background-color: #f5f5f5;");

    // ===== 顶部卡片：Logo和标题 =====
    QWidget *headerCard = new QWidget();
    headerCard->setStyleSheet(
        "background-color: #ffffff;"
        "border-radius: 8px;"
        "border-left: 4px solid #ff9500;"
    );
    headerCard->setFixedHeight(100);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    
    QLabel *logoLabel = new QLabel("📝");
    logoLabel->setStyleSheet("font-size: 48px;");
    headerLayout->addWidget(logoLabel);
    
    QVBoxLayout *titleLayout = new QVBoxLayout();
    m_titleLabel = new QLabel("注册新账号");
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #ff9500;");
    titleLayout->addWidget(m_titleLabel);
    
    QLabel *subtitleLabel = new QLabel("加入FlyOver航班预订平台");
    subtitleLabel->setStyleSheet("font-size: 12px; color: #999;");
    titleLayout->addWidget(subtitleLabel);
    
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addWidget(headerCard);

    mainLayout->addSpacing(10);

    // ===== 用户名输入框 =====
    QLabel *accountLabel = new QLabel("👤 用户名");
    accountLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(accountLabel);
    m_accountEdit = new QLineEdit();
    m_accountEdit->setPlaceholderText("4-20位字母、数字或下划线");
    m_accountEdit->setMinimumHeight(42);
    m_accountEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  color: #333;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #0078d4;"
        "  background-color: #f9f9f9;"
        "}"
    );
    mainLayout->addWidget(m_accountEdit);

    // ===== 邮箱输入框 =====
    QLabel *emailLabel = new QLabel("📧 邮箱");
    emailLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(emailLabel);
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("请输入有效的邮箱地址");
    m_emailEdit->setMinimumHeight(42);
    m_emailEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  color: #333;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #0078d4;"
        "  background-color: #f9f9f9;"
        "}"
    );
    mainLayout->addWidget(m_emailEdit);

    // ===== 手机号输入框 =====
    QLabel *phoneLabel = new QLabel("☎️ 手机号");
    phoneLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(phoneLabel);
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("请输入11位手机号码");
    m_phoneEdit->setMinimumHeight(42);
    m_phoneEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  color: #333;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #0078d4;"
        "  background-color: #f9f9f9;"
        "}"
    );
    mainLayout->addWidget(m_phoneEdit);

    // ===== 密码输入框 =====
    QLabel *passwordLabel = new QLabel("🔐 密码");
    passwordLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(passwordLabel);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("6-20位，包含大小写字母、数字和符号");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setMinimumHeight(42);
    m_passwordEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  color: #333;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #0078d4;"
        "  background-color: #f9f9f9;"
        "}"
    );
    mainLayout->addWidget(m_passwordEdit);

    // ===== 确认密码输入框 =====
    QLabel *confirmLabel = new QLabel("🔒 确认密码");
    confirmLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(confirmLabel);
    m_confirmEdit = new QLineEdit();
    m_confirmEdit->setPlaceholderText("请再次输入密码");
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setMinimumHeight(42);
    m_confirmEdit->setStyleSheet(
        "QLineEdit {"
        "  background-color: #ffffff;"
        "  color: #333;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #0078d4;"
        "  background-color: #f9f9f9;"
        "}"
    );
    mainLayout->addWidget(m_confirmEdit);

    // ===== 同意协议 =====
    QHBoxLayout *agreeLayout = new QHBoxLayout();
    m_agreeCheckBox = new QCheckBox("我已阅读并同意");
    m_agreeCheckBox->setStyleSheet(
        "QCheckBox { color: #666; font-size: 12px; spacing: 6px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; }"
        "QCheckBox::indicator:unchecked { border: 1px solid #ddd; background-color: #fff; }"
        "QCheckBox::indicator:checked { background-color: #0078d4; border: 1px solid #0078d4; }"
    );
    agreeLayout->addWidget(m_agreeCheckBox);
    
    QPushButton *serviceLink = new QPushButton("《服务条款》");
    serviceLink->setStyleSheet(
        "QPushButton {"
        "  color: #0078d4;"
        "  background: transparent;"
        "  border: none;"
        "  font-size: 12px;"
        "  text-decoration: underline;"
        "  padding: 0px;"
        "}"
        "QPushButton:hover { color: #106ebe; }"
    );
    serviceLink->setCursor(Qt::PointingHandCursor);
    agreeLayout->addWidget(serviceLink);
    agreeLayout->addStretch();
    mainLayout->addLayout(agreeLayout);

    mainLayout->addSpacing(8);

    // ===== 注册按钮 =====
    m_registerBtn = new QPushButton("✓ 立即注册");
    m_registerBtn->setMinimumHeight(46);
    m_registerBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #0078d4;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #106ebe; }"
        "QPushButton:pressed { background-color: #005a9e; }"
    );
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    connect(m_registerBtn, &QPushButton::clicked, this, &RegisterDialog::on_registerButton_clicked);
    mainLayout->addWidget(m_registerBtn);

    mainLayout->addSpacing(10);

    // ===== 取消按钮 =====
    m_cancelBtn = new QPushButton("取消");
    m_cancelBtn->setMinimumHeight(42);
    m_cancelBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #f0f0f0;"
        "  color: #333;"
        "  border: 1px solid #ddd;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #e8e8e8; }"
    );
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(m_cancelBtn, &QPushButton::clicked, this, &RegisterDialog::on_cancelButton_clicked);
    mainLayout->addWidget(m_cancelBtn);

    mainLayout->addStretch();

    // ===== 底部版权 =====
    QLabel *footerLabel = new QLabel("© 2025 FlyOver. All rights reserved.");
    footerLabel->setStyleSheet("color: #999; font-size: 10px;");
    footerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footerLabel);
}void RegisterDialog::applyDarkTheme()
{
    // 注册界面不需要dark主题，保持light主题即可
    applyLightTheme();
}

void RegisterDialog::applyLightTheme()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLineEdit {
            background-color: #ffffff;
            color: #333333;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 8px;
            font-size: 12px;
        }
        QLineEdit:focus {
            border: 2px solid #0078d4;
            background-color: #f9f9f9;
        }
        QPushButton {
            border-radius: 6px;
            font-weight: bold;
            padding: 8px 16px;
        }
        QPushButton#registerBtn {
            background-color: #0078d4;
            color: white;
        }
        QPushButton#registerBtn:hover {
            background-color: #106ebe;
        }
        QPushButton#cancelBtn {
            background-color: #f0f0f0;
            color: #333;
            border: 1px solid #ddd;
        }
        QPushButton#cancelBtn:hover {
            background-color: #e8e8e8;
        }
        QCheckBox {
            color: #555;
            spacing: 5px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border-radius: 3px;
            border: 1px solid #ddd;
            background-color: #ffffff;
        }
        QCheckBox::indicator:checked {
            background-color: #0078d4;
        }
        QLabel {
            color: #333;
        }
    )");
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #0078d4;");
    m_registerBtn->setObjectName("registerBtn");
    m_cancelBtn->setObjectName("cancelBtn");
}

void RegisterDialog::applyTheme(bool isDarkMode)
{
    m_isDarkMode = isDarkMode;
    if (isDarkMode) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
}

bool RegisterDialog::validateInput()
{
    QString account = m_accountEdit->text().trimmed();
    QString email = m_emailEdit->text().trimmed();
    QString phone = m_phoneEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString confirm = m_confirmEdit->text();

    if (account.isEmpty() || email.isEmpty() || phone.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "所有字段均不能为空！");
        return false;
    }

    if (account.length() < 4 || account.length() > 20) {
        QMessageBox::warning(this, "验证失败", "用户名长度应为4-20位！");
        return false;
    }

    if (phone.length() != 11 || !phone.at(0).isDigit()) {
        QMessageBox::warning(this, "验证失败", "请输入有效的11位手机号码！");
        return false;
    }

    QRegularExpression emailRegex("^[^@]+@[^@]+\\.[^@]+$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "验证失败", "请输入有效的邮箱地址！");
        return false;
    }

    if (password.length() < 6 || password.length() > 20) {
        QMessageBox::warning(this, "验证失败", "密码长度应为6-20位！");
        return false;
    }

    if (password != confirm) {
        QMessageBox::warning(this, "验证失败", "两次输入的密码不一致！");
        return false;
    }

    if (!m_agreeCheckBox->isChecked()) {
        QMessageBox::warning(this, "验证失败", "请同意服务条款！");
        return false;
    }

    return true;
}

bool RegisterDialog::registerUser(const QString &account, const QString &password)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "错误", "数据库连接失败！");
        return false;
    }

    QSqlQuery query(db);
    query.prepare("SELECT account FROM users WHERE account = :account");
    query.bindValue(":account", account);

    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return false;
    }

    if (query.next()) {
        QMessageBox::warning(this, "注册失败", "该用户名已被注册！");
        return false;
    }

    query.prepare("INSERT INTO users (account, password) VALUES (:account, :password)");
    query.bindValue(":account", account);
    // 注意：实际应用中，您应该对密码进行哈希处理，这里保留原样以符合代码功能
    query.bindValue(":password", password);

    if (!query.exec()) {
        QMessageBox::critical(this, "注册失败", "插入数据失败：" + query.lastError().text());
        return false;
    }

    return true;
} // <-- 修复：补上 registerUser 函数的结束花括号

void RegisterDialog::on_registerButton_clicked()
{
    if (!validateInput()) {
        return;
    }

    QString account = m_accountEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (registerUser(account, password)) {
        QMessageBox::information(this, "注册成功", "账号注册成功，请返回登录！");
        close();
    }
}

void RegisterDialog::on_cancelButton_clicked()
{
    close();
}
