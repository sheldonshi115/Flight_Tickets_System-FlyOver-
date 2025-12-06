#include "login.h"
#include "ui_login.h"
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

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(nullptr),
    registerDialog(nullptr),
    m_isDarkMode(false)
{
    setWindowTitle("FlyOver - 登陆");
    setFixedSize(450, 550);
    setWindowIcon(QIcon(":/images/logo.png"));
    
    setupUI();
    applyLightTheme();
}

void LoginDialog::setupUI()
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
        "border-left: 4px solid #0078d4;"
    );
    headerCard->setFixedHeight(100);
    QHBoxLayout *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    
    QLabel *logoLabel = new QLabel("✈️");
    logoLabel->setStyleSheet("font-size: 48px;");
    headerLayout->addWidget(logoLabel);
    
    QVBoxLayout *titleLayout = new QVBoxLayout();
    m_titleLabel = new QLabel("FlyOver");
    m_titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #0078d4;");
    titleLayout->addWidget(m_titleLabel);
    
    QLabel *subtitleLabel = new QLabel("航班票务预订系统");
    subtitleLabel->setStyleSheet("font-size: 12px; color: #999;");
    titleLayout->addWidget(subtitleLabel);
    
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addWidget(headerCard);

    mainLayout->addSpacing(10);

    // ===== 账号输入框 =====
    QLabel *accountLabel = new QLabel("👤 账号");
    accountLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(accountLabel);
    
    m_accountEdit = new QLineEdit();
    m_accountEdit->setPlaceholderText("请输入用户名或邮箱");
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

    // ===== 密码输入框 =====
    QLabel *passwordLabel = new QLabel("🔐 密码");
    passwordLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; margin-left: 8px;");
    mainLayout->addWidget(passwordLabel);
    
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("请输入密码");
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

    // ===== 选项行：记住我 和 忘记密码 =====
    QHBoxLayout *optionLayout = new QHBoxLayout();
    m_rememberMe = new QCheckBox("记住我");
    m_rememberMe->setStyleSheet(
        "QCheckBox { color: #666; font-size: 12px; spacing: 6px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 3px; }"
        "QCheckBox::indicator:unchecked { border: 1px solid #ddd; background-color: #fff; }"
        "QCheckBox::indicator:checked { background-color: #0078d4; border: 1px solid #0078d4; }"
    );
    optionLayout->addWidget(m_rememberMe);
    
    QPushButton *forgetBtn = new QPushButton("忘记密码？");
    forgetBtn->setStyleSheet(
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
    forgetBtn->setCursor(Qt::PointingHandCursor);
    optionLayout->addStretch();
    optionLayout->addWidget(forgetBtn);
    mainLayout->addLayout(optionLayout);

    mainLayout->addSpacing(8);

    // ===== 登录按钮 =====
    m_loginBtn = new QPushButton("🔓 登 陆");
    m_loginBtn->setMinimumHeight(46);
    m_loginBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #0078d4;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #106ebe;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #005a9e;"
        "}"
    );
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    connect(m_loginBtn, &QPushButton::clicked, this, &LoginDialog::on_loginButton_clicked);
    mainLayout->addWidget(m_loginBtn);

    mainLayout->addSpacing(12);

    // ===== 注册链接 =====
    QHBoxLayout *registerLayout = new QHBoxLayout();
    QLabel *tipLabel = new QLabel("还没有账号？");
    tipLabel->setStyleSheet("color: #666; font-size: 12px;");
    registerLayout->addWidget(tipLabel);
    
    m_registerLink = new QPushButton("立即注册");
    m_registerLink->setStyleSheet(
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
    m_registerLink->setCursor(Qt::PointingHandCursor);
    connect(m_registerLink, &QPushButton::clicked, this, &LoginDialog::on_registerLink_clicked);
    registerLayout->addWidget(m_registerLink);
    registerLayout->addStretch();
    mainLayout->addLayout(registerLayout);

    mainLayout->addStretch();

    // ===== 底部版权信息 =====
    QLabel *footerLabel = new QLabel("© 2025 FlyOver. All rights reserved.");
    footerLabel->setStyleSheet("color: #999; font-size: 10px;");
    footerLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(footerLabel);
}

void LoginDialog::applyDarkTheme()
{
    // 登录界面不需要dark主题，保持light主题即可
    applyLightTheme();
}

void LoginDialog::applyLightTheme()
{
    // 浅色主题（登录界面统一风格，不需要额外样式覆盖）
    setStyleSheet("QDialog { background-color: #f5f5f5; }");
}

void LoginDialog::applyTheme(bool isDarkMode)
{
    m_isDarkMode = isDarkMode;
    if (isDarkMode) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
}

LoginDialog::~LoginDialog()
{
    if (registerDialog) delete registerDialog;
}

void LoginDialog::on_loginButton_clicked()
{
    QString account = m_accountEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "账号和密码不能为空！");
        return;
    }

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT password FROM users WHERE account = :account");
    query.bindValue(":account", account);

    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }

    if (query.next()) {
        if (query.value(0).toString() == password) {
            QMessageBox::information(this, "成功", "登录成功！");
            accept();
        } else {
            QMessageBox::warning(this, "失败", "密码错误！");
        }
    } else {
        QMessageBox::warning(this, "失败", "账号不存在！");
    }
}

void LoginDialog::on_registerLink_clicked()
{
    if (!registerDialog) {
        registerDialog = new RegisterDialog(this);
        registerDialog->setWindowTitle("注册");
    }
    registerDialog->exec();
}

void LoginDialog::onRememberMeToggled(bool checked)
{
    // 可以在这里实现记住我功能的逻辑
    Q_UNUSED(checked);
}
