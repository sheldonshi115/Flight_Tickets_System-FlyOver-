#include "dialog_login.h"
#include "ui_dialog_login.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
Dialog_login::Dialog_login(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog_login)
{
    ui->setupUi(this);
    setWindowTitle("注册账号");
}

Dialog_login::~Dialog_login()
{
    delete ui;
}
QString Dialog_login::getAccount()const{
    return ui->countValue->text();
}
QString Dialog_login::getPwd() const{
    return ui->passwordValue->text();
}
void Dialog_login::on_login_button_clicked(){
    QString user_account=getAccount();
    QString user_password=getPwd();
    if (user_account.isEmpty() || user_password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "账号和密码不能为空！");
        return;
    }

    // 3. 连接本地MySQL数据库（使用约定的配置）
    // 如果已存在连接，直接使用；否则新建连接
    QSqlDatabase db;
    db = QSqlDatabase::addDatabase("QODBC"); // 指定MySQL驱动
    //database config
    db.setHostName("localhost");
    db.setDatabaseName("login_db");
    db.setUserName("root");
    db.setPassword("yourpassword");     //填你的密码
    db.setPort(3306);                  // MySQL默认端口

    // 检查连接是否成功
    if (!db.open()) {
        QMessageBox::critical(this, "数据库连接失败",
                              "错误原因：" + db.lastError().text() + "\n请检查本地MySQL是否启动，以及账号密码是否正确（约定密码123456）");
        return; // 连接失败，退出函数
    }

    // 4. 检查数据库中是否已存在相同账号（参数化查询，防SQL注入）
    QSqlQuery query; // 创建查询对象
    // 准备查询SQL：查找account等于输入账号的记录
    bool isCheckOk = query.prepare("SELECT account FROM user WHERE account = :account");
    if (!isCheckOk) {
        QMessageBox::critical(this, "查询失败", "SQL语句错误：" + query.lastError().text());
        db.close(); // 关闭连接
        return;
    }
    query.bindValue(":account", user_account); // 绑定账号参数
    if (!query.exec()) { // 执行查询
        QMessageBox::critical(this, "查询失败", "执行查询错误：" + query.lastError().text());
        db.close();
        return;
    }

    // 5. 根据查询结果判断是否注册
    if (query.next()) { // 查到记录 → 账号已存在
        QMessageBox::information(this, "注册失败", "账号已存在，请更换账号！");
    } else { // 未查到记录 → 插入新账号
        // 准备插入SQL：添加新用户到user表
        bool isInsertOk = query.prepare("INSERT INTO user (account, password) VALUES (:account, :password)");
        if (!isInsertOk) {
            QMessageBox::critical(this, "插入失败", "SQL语句错误：" + query.lastError().text());
            db.close();
            return;
        }
        query.bindValue(":account", user_account);   // 绑定账号
        query.bindValue(":password", user_password); // 绑定密码
        if (query.exec()) { // 执行插入
            QMessageBox::information(this, "注册成功", "账号注册成功！你的账号是：" + user_account);
            accept();
        } else {
            QMessageBox::critical(this, "插入失败", "添加账号错误：" + query.lastError().text());
        }
    }

    // 6. 关闭数据库连接（释放资源）
    db.close();
}
void Dialog_login::on_login_rejected_clicked(){
    reject();
}
