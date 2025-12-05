#ifndef DBMANAGER_H
#define DBMANAGER_H
#include<order.h>
#include<QSqlError>
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
     bool addOrder(const Order& order);

    // 初始化数据库（创建表）
    bool initDatabase();

    // 航班操作方法
    QList<Flight> getAllFlights();
    QList<Flight> findFlights(const QString& departure, const QString& arrival, const QDateTime& date);
    QSqlDatabase getDatabase() { return db; }
    bool addFlight(const Flight& flight);
    bool updateFlight(const Flight& flight);
    bool removeFlight(int flightId);

    bool addUser(const QString& account, const QString& password, const QString& role = "user");
    bool verifyUser(const QString& account, const QString& password, QString& role);
    QList<Order> getAllOrders(); // 获取所有订单
    QList<Order> findOrders(const QString& flightNum, const QDate& date, const QString& status); // 筛选订单
     // 新增订单
    bool cancelOrder(int orderId); // 取消订单（更新状态为"已取消"）
    Order getOrderById(int orderId); // 根据ID获取订单详情
    Flight getFlightByFlightNum(const QString& flightNum);
    // 新增：获取最后一次数据库错误信息（调试用）
    QString lastError() const { return db.lastError().text(); }
};

#endif // DBMANAGER_H
