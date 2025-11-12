// dbmanager.cpp
#include "dbmanager.h"
#include <QMessageBox>

// 数据库配置（根据实际情况修改）
const QString DBManager::DB_NAME = "flight_ticket_db"; // 先在本机创建一个这样名字的数据库
const QString DBManager::DB_HOST = "localhost";
const QString DBManager::DB_USER = "root";
const QString DBManager::DB_PWD = ""; // 替换为你的MySQL密码
const int DBManager::DB_PORT = 3306;

QSqlDatabase DBManager::getDatabase() {
    // 避免重复创建连接
    if (QSqlDatabase::contains("flight_conn")) {
        return QSqlDatabase::database("flight_conn");
    }

    // 创建MySQL连接（若用SQLite，替换为QSQLITE，无需主机/端口）
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(DB_NAME);
    db.setHostName(DB_HOST);
    db.setUserName(DB_USER);
    db.setPassword(DB_PWD);
    db.setPort(DB_PORT);

    // 尝试打开连接
    if (!db.open()) {
        QMessageBox::critical(nullptr, "数据库连接失败",
                              "错误信息：" + db.lastError().text());
        return QSqlDatabase();
    }
    return db;
}

bool DBManager::initDatabase() {
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) return false;

    // 创建users表
    QSqlQuery query(db);
    QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT PRIMARY KEY AUTO_INCREMENT,
            account VARCHAR(50) NOT NULL UNIQUE,
            password VARCHAR(100) NOT NULL,
            role VARCHAR(20) DEFAULT 'user'
        )
    )";
    // 创建flights表
    QString createFlights = R"(
        CREATE TABLE IF NOT EXISTS flights (
            id INT PRIMARY KEY AUTO_INCREMENT,
            flight_num VARCHAR(20) NOT NULL UNIQUE,
            departure VARCHAR(50) NOT NULL,
            destination VARCHAR(50) NOT NULL,
            depart_time DATETIME NOT NULL,
            arrive_time DATETIME NOT NULL,
            seat_count INT NOT NULL,
            price DECIMAL(10,2) NOT NULL
        )
    )";
    // 同理创建tickets、orders表（代码略）

    // 执行建表语句
    return query.exec(createUsers) && query.exec(createFlights);
}
