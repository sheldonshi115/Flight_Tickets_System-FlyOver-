// dbmanager.h
#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>

class DBManager {
public:
    // 单例模式：获取数据库实例
    static QSqlDatabase getDatabase();
    // 初始化数据库（创建表）
    static bool initDatabase();

private:
    // 私有构造，禁止外部实例化
    DBManager() = default;
    // 数据库配置
    static const QString DB_NAME;
    static const QString DB_HOST;
    static const QString DB_USER;
    static const QString DB_PWD;
    static const int DB_PORT;
};

#endif // DBMANAGER_H
