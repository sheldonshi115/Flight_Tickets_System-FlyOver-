#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QtMath>
#include "flight.h" // 引入 Flight 类

class DBManager : public QObject
{
    Q_OBJECT

private:
    // 单例模式：私有构造函数
    explicit DBManager(QObject *parent = nullptr);
    QSqlDatabase db;

    // 数据库配置参数
    static const QString DB_NAME;
    static const QString DB_HOST;
    static const QString DB_USER;
    static const QString DB_PWD;
    static const int DB_PORT;
    void insertTestFlights();

public:
    // 单例模式：获取唯一实例
    static DBManager &instance();

    // 初始化数据库（创建表）
    bool initDatabase();

    // 航班操作方法
    QList<Flight> getAllFlights();
    QList<Flight> findFlights(const QString& departure, const QString& arrival, const QDateTime& date);
    QSqlDatabase getDatabase() { return db; }
    bool addFlight(const Flight& flight);
    bool updateFlight(const Flight& flight);
    bool removeFlight(int flightId);
};

#endif // DBMANAGER_H
