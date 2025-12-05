#include "register.h"
#include "ui_register.h"
#include "dbmanager.h" // 引入DBManager
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    setWindowTitle("注册");
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->confirmEdit->setEchoMode(QLineEdit::Password);
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_registerButton_clicked()
{
    QString account = ui->accountEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmEdit->text();

    // 输入验证
    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "账号和密码不能为空！");
        return;
    }
    if (password != confirm) {
        QMessageBox::warning(this, "提示", "两次密码输入不一致！");
        return;
    }
    if (account.length() < 4 || password.length() < 6) {
        QMessageBox::warning(this, "提示", "账号至少4位，密码至少6位！");
        return;
    }

    // 通过单例实例调用DBManager获取数据库连接
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        return; // DBManager已处理连接失败
    }

    // 检查账号是否存在
    QSqlQuery query(db);
    query.prepare("SELECT account FROM users WHERE account = :account"); // 表名是users
    query.bindValue(":account", account);
    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }

    if (query.next()) {
        QMessageBox::warning(this, "失败", "该账号已被注册！");
        return;
    }

    // 插入新用户
    query.prepare("INSERT INTO users (account, password) VALUES (:account, :password)"); // 表名是users
    query.bindValue(":account", account);
    query.bindValue(":password", password); // 建议加密：QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Md5).toHex()

    if (query.exec()) {
        QMessageBox::information(this, "成功", "注册成功，请登录！");
        close();
    } else {
        QMessageBox::critical(this, "失败", "注册失败：" + query.lastError().text());
    }
}

void RegisterDialog::on_cancelButton_clicked()
{
    close();
}
