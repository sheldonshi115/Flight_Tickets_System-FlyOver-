#include "login.h"
#include "ui_login.h"
#include "dbmanager.h" // 引入DBManager
#include "mainwindow.h"
#include "forgotpassworddialog.h" // 新增：忘记密码对话框
#include "utils.h"
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
    registerDialog(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("登录");

    // 注册链接样式
    ui->registerLink->setStyleSheet("color: #0000FF; text-decoration: underline; background: transparent; border: none;");
    ui->registerLink->setText("没有账号？点击注册");
    
    // 新增：忘记密码链接
    ui->forgotPasswordLink->setStyleSheet("color: #FF8C00; text-decoration: underline; background: transparent; border: none;");
    ui->forgotPasswordLink->setText("忘记密码？");
    
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    
    // 连接忘记密码链接信号
    connect(ui->forgotPasswordLink, &QPushButton::clicked, this, &LoginDialog::onForgotPasswordClicked);
}

LoginDialog::~LoginDialog()
{
    delete ui;
    //if (registerDialog) delete registerDialog;
}

void LoginDialog::on_loginButton_clicked()
{
    QString account = ui->accountEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    // 输入验证
    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "账号和密码不能为空！");
        return;
    }

    // 新增：账号格式校验（与注册逻辑一致）
    if (account.length() < 4 || account.length() > 20 ||
        !account.contains(QRegularExpression("^[A-Za-z0-9]+$"))) {
        QMessageBox::warning(this, "提示", "账号需为4-20位字母或数字！");
        return;
    }
    // 新增：登录锁定检查
    if (QDateTime::currentDateTime() < m_lockUntil) {
        int left = QDateTime::currentDateTime().secsTo(m_lockUntil);
        QMessageBox::warning(this, "锁定中", QString("连续输错5次，还需等待%1秒").arg(left));
        return;
    }

    // 直接调用DBManager获取数据库连接
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        return; // DBManager已弹出连接失败提示，无需重复处理
    }

    // 验证账号密码
    QSqlQuery query(db);
    query.prepare("SELECT password, salt FROM users WHERE account = :account"); // 注意表名是users（DBManager中创建的是users）
    query.bindValue(":account", account);

    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }

    if (query.next()) {
        // 新增：加密比对
        QString storedPwd = query.value(0).toString();
        QString salt = query.value(1).toString();

        // 处理盐值为空的情况（如旧用户）
        if (salt.isEmpty()) {
            QMessageBox::warning(this, "错误", "账号信息不完整，请联系管理员重置密码！");
            return;
        }

        QString inputPwd = hashPassword(password, salt);

        if (inputPwd == storedPwd) {
            // 登录成功
            QMessageBox::information(this, "成功", "登录成功！");
            m_failedAttempts = 0; // 重置失败次数

            // 从数据库加载用户信息
            UserProfile userProfile = DBManager::instance().loadUserProfile(account);
            qDebug() << "登录成功，加载的账号:" << userProfile.account;
            
            // 创建主窗口
            MainWindow *mainWin = new MainWindow();
            mainWin->setAttribute(Qt::WA_DeleteOnClose);
            // 将加载的用户信息传递给主窗口
            mainWin->setUserProfile(userProfile);
            mainWin->show();
            
            // 关闭登录窗口
            QTimer::singleShot(100, this, &LoginDialog::close);
        } else {
            // 密码错误（新增失败次数限制）
            m_failedAttempts++;
            int remaining = 5 - m_failedAttempts;
            if (remaining <= 0) {
                m_lockUntil = QDateTime::currentDateTime().addSecs(60);
                QMessageBox::warning(this, "失败", "密码错误，账号已锁定1分钟！");
            } else {
                QMessageBox::warning(this, "失败", QString("密码错误，还可尝试%1次").arg(remaining));
            }
            ui->passwordEdit->clear();
        }
    } else {
        QMessageBox::warning(this, "失败", "账号不存在！");
        ui->accountEdit->clear();
        ui->passwordEdit->clear();
    }
}
QString LoginDialog::get_account()const{
    return ui->accountEdit->text().trimmed();
}
void LoginDialog::on_registerLink_clicked()
{
    if (!registerDialog) {
        registerDialog.reset(new RegisterDialog(this));
        registerDialog->setWindowTitle("注册");
    }
    registerDialog->exec();
}

void LoginDialog::onForgotPasswordClicked()
{
    ForgotPasswordDialog forgotDialog(this);
    forgotDialog.exec();
}

