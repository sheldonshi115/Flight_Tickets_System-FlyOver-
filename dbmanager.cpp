#include "dbmanager.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

// 初始化静态常量
const QString DBManager::DB_NAME = "flight_ticket_db";
const QString DBManager::DB_HOST = "localhost";
const QString DBManager::DB_USER = "root";
const QString DBManager::DB_PWD = "cai2166013";
const int DBManager::DB_PORT = 3306;

// 单例模式：静态实例
DBManager& DBManager::instance()
{
    static DBManager instance;
    return instance;
}

// 私有构造函数：初始化数据库连接
DBManager::DBManager(QObject *parent) : QObject(parent)
{
    // 避免重复创建连接
    if (QSqlDatabase::contains("flight_conn")) {
        db = QSqlDatabase::database("flight_conn");
    } else {
        // 创建MySQL连接
        db = QSqlDatabase::addDatabase("QODBC", "flight_conn");
        db.setDatabaseName(QString("DRIVER={MySQL ODBC 8.0 ANSI Driver};SERVER=%1;DATABASE=%2;UID=%3;PWD=%4;PORT=%5;")
                               .arg(DB_HOST).arg(DB_NAME).arg(DB_USER).arg(DB_PWD).arg(DB_PORT));
        // 注意：你需要确保系统安装了对应版本的MySQL ODBC驱动，并在上面的字符串中正确指定

        // 尝试打开连接
        if (!db.open()) {
            QMessageBox::critical(nullptr, "数据库连接失败",
                                  "错误信息：" + db.lastError().text());
        } else {
            qDebug() << "数据库连接成功！";
            initDatabase(); // 自动初始化数据库表
        }
    }
}

// 初始化数据库表
bool DBManager::initDatabase()
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT PRIMARY KEY AUTO_INCREMENT,
            account VARCHAR(50) NOT NULL UNIQUE,
            password VARCHAR(100) NOT NULL,
            role VARCHAR(20) DEFAULT 'user'
        )
    )";
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
    // 执行建表语句
    if(query.exec(createUsers) && query.exec(createFlights)) {
        qDebug() << "数据表检查/创建成功。";
        return true;
    } else {
        qWarning() << "创建数据表失败:" << query.lastError().text();
        return false;
    }
}

// 获取所有航班
QList<Flight> DBManager::getAllFlights()
{
    QList<Flight> flights;
    if (!db.isOpen()) return flights;

    QSqlQuery query(db);
    if (query.exec("SELECT id, flight_num, departure, destination, depart_time, arrive_time, seat_count, price FROM flights ORDER BY depart_time")) {
        while (query.next()) {
            Flight flight;
            flight.setId(query.value("id").toInt());
            flight.setFlightNumber(query.value("flight_num").toString());
            flight.setDepartureCity(query.value("departure").toString());
            flight.setArrivalCity(query.value("destination").toString());
            flight.setDepartureTime(query.value("depart_time").toDateTime());
            flight.setArrivalTime(query.value("arrive_time").toDateTime());
            flight.setTotalSeats(query.value("seat_count").toInt());
            flight.setPrice(query.value("price").toDouble());
            // availableSeats 在数据库中没有字段，这里可以根据业务逻辑计算，
            // 例如：可用座位 = 总座位 - 已售出座位(需要查询orders表)
            // 为简化，这里暂时设为总座位
            flight.setAvailableSeats(query.value("seat_count").toInt());

            flights.append(flight);
        }
    } else {
        qWarning() << "查询所有航班失败:" << query.lastError().text();
    }
    return flights;
}

// 查找航班
QList<Flight> DBManager::findFlights(const QString& departure, const QString& arrival, const QDateTime& date)
{
    QList<Flight> flights;
    if (!db.isOpen()) return flights;

    QString sql = "SELECT id, flight_num, departure, destination, depart_time, arrive_time, seat_count, price FROM flights WHERE 1=1";
    if (!departure.isEmpty()) {
        sql += " AND departure LIKE :departure";
    }
    if (!arrival.isEmpty()) {
        sql += " AND destination LIKE :arrival";
    }
    if (date.isValid()) {
        sql += " AND DATE(depart_time) = :date";
    }
    sql += " ORDER BY depart_time";

    QSqlQuery query(db);
    query.prepare(sql);
    if (!departure.isEmpty()) {
        query.bindValue(":departure", "%" + departure + "%");
    }
    if (!arrival.isEmpty()) {
        query.bindValue(":arrival", "%" + arrival + "%");
    }
    if (date.isValid()) {
        query.bindValue(":date", date.date().toString("yyyy-MM-dd"));
    }

    if (query.exec()) {
        while (query.next()) {
            Flight flight;
            flight.setId(query.value("id").toInt());
            flight.setFlightNumber(query.value("flight_num").toString());
            flight.setDepartureCity(query.value("departure").toString());
            flight.setArrivalCity(query.value("destination").toString());
            flight.setDepartureTime(query.value("depart_time").toDateTime());
            flight.setArrivalTime(query.value("arrive_time").toDateTime());
            flight.setTotalSeats(query.value("seat_count").toInt());
            flight.setPrice(query.value("price").toDouble());
            flight.setAvailableSeats(query.value("seat_count").toInt()); // 同样，这里是简化处理

            flights.append(flight);
        }
    } else {
        qWarning() << "查找航班失败:" << query.lastError().text();
    }
    return flights;
}

// 添加航班
bool DBManager::addFlight(const Flight& flight)
{
    if (!db.isOpen()) return false;

    QString sql = R"(
        INSERT INTO flights (flight_num, departure, destination, depart_time, arrive_time, seat_count, price)
        VALUES (:flight_num, :departure, :destination, :depart_time, :arrive_time, :seat_count, :price)
    )";

    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":flight_num", flight.flightNumber());
    query.bindValue(":departure", flight.departureCity());
    query.bindValue(":destination", flight.arrivalCity());
    query.bindValue(":depart_time", flight.departureTime());
    query.bindValue(":arrive_time", flight.arrivalTime());
    query.bindValue(":seat_count", flight.totalSeats());
    query.bindValue(":price", flight.price());

    if (query.exec()) {
        qDebug() << "添加航班成功。";
        return true;
    } else {
        qWarning() << "添加航班失败:" << query.lastError().text();
        return false;
    }
}

// 更新航班
bool DBManager::updateFlight(const Flight& flight)
{
    if (!db.isOpen() || flight.id() == 0) return false;

    QString sql = R"(
        UPDATE flights SET
        flight_num = :flight_num,
        departure = :departure,
        destination = :destination,
        depart_time = :depart_time,
        arrive_time = :arrive_time,
        seat_count = :seat_count,
        price = :price
        WHERE id = :id
    )";

    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":flight_num", flight.flightNumber());
    query.bindValue(":departure", flight.departureCity());
    query.bindValue(":destination", flight.arrivalCity());
    query.bindValue(":depart_time", flight.departureTime());
    query.bindValue(":arrive_time", flight.arrivalTime());
    query.bindValue(":seat_count", flight.totalSeats());
    query.bindValue(":price", flight.price());
    query.bindValue(":id", flight.id());

    if (query.exec()) {
        qDebug() << "更新航班成功。";
        return true;
    } else {
        qWarning() << "更新航班失败:" << query.lastError().text();
        return false;
    }
}

// 删除航班
bool DBManager::removeFlight(int flightId)
{
    if (!db.isOpen() || flightId == 0) return false;

    QString sql = "DELETE FROM flights WHERE id = :id";

    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":id", flightId);

    if (query.exec()) {
        qDebug() << "删除航班成功。";
        return true;
    } else {
        qWarning() << "删除航班失败:" << query.lastError().text();
        return false;
    }
}
