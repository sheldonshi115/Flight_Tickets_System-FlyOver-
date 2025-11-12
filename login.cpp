#include "login.h"
#include "ui_login.h"
#include "dbmanager.h" // 引入DBManager
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
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}

LoginDialog::~LoginDialog()
{
    delete ui;
    if (registerDialog) delete registerDialog;
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

    // 直接调用DBManager获取数据库连接
    QSqlDatabase db = DBManager::getDatabase();
    if (!db.isOpen()) {
        return; // DBManager已弹出连接失败提示，无需重复处理
    }

    // 验证账号密码
    QSqlQuery query(db);
    query.prepare("SELECT password FROM users WHERE account = :account"); // 注意表名是users（DBManager中创建的是users）
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
