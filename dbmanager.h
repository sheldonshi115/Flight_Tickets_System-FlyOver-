#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QtMath>
#include <QVariantMap>  // 新增：用于存储用户个人资料（昵称+头像路径）
#include <QDateTime>    // 新增：确保QDateTime类型可用
#include "flight.h" // 引入 Flight 类

class DBManager : public QObject
{
    Q_OBJECT

private:
    // 单例模式：私有构造函数
    explicit DBManager(QObject *parent = nullptr);
    QSqlDatabase db;

    // 数据库配置参数（静态常量）
    static const QString DB_NAME;
    static const QString DB_HOST;
    static const QString DB_USER;
    static const QString DB_PWD;
    static const int DB_PORT;

    // 内部方法：插入测试航班数据
    void insertTestFlights();

public:
    // 单例模式：获取唯一实例
    static DBManager &instance();

    // 初始化数据库（创建表）
    bool initDatabase();

    // -------------------------- 航班操作方法 --------------------------
    // 获取所有航班（按出发时间排序）
    QList<Flight> getAllFlights();
    // 条件查询航班（出发地/到达地/日期）
    QList<Flight> findFlights(const QString& departure, const QString& arrival, const QDateTime& date);
    // 获取数据库连接（供登录/注册等外部模块使用）
    QSqlDatabase getDatabase() { return db; }
    // 添加单个航班
    bool addFlight(const Flight& flight);
    // 更新航班信息（按ID定位）
    bool updateFlight(const Flight& flight);
    // 删除航班（按ID删除）
    bool removeFlight(int flightId);

    // -------------------------- 用户资料操作方法 --------------------------
    // 获取用户个人资料（昵称+头像路径）
    QVariantMap getUserProfile(const QString& account);
    // 更新用户个人资料（昵称+头像路径）
    bool updateUserProfile(const QString& account, const QString& nickname, const QString& avatarPath);
};

#endif // DBMANAGER_H
