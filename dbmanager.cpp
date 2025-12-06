#include "dbmanager.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <ctime>
#include <cstdlib>

// 使用你的数据库配置
const QString DBManager::DB_NAME = "flight_ticket_db";
const QString DBManager::DB_HOST = "localhost";
const QString DBManager::DB_USER = "root";       // 你的MySQL用户名
const QString DBManager::DB_PWD = "cai2166013"; // 你的MySQL密码
const int DBManager::DB_PORT = 3306;             // 默认端口

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

// 插入测试航班数据（包含available_seats）
void DBManager::insertTestFlights()
{
    if (!db.isOpen()) {
        qWarning() << "数据库未连接，无法插入测试数据！";
        return;
    }

    QSqlQuery query(db);
    // 先检查是否有任何航班数据
    query.exec("SELECT COUNT(*) FROM flights");
    query.next();
    if (query.value(0).toInt() > 0) {
        qDebug() << "数据库已有航班数据，跳过插入测试数据";
        return;  // 已有数据，不再插入
    }
    query.prepare(R"(
        INSERT INTO flights (flight_num, departure, destination, depart_time, arrive_time, seat_count, available_seats, price)
        VALUES (:flight_num, :departure, :destination, :depart_time, :arrive_time, :seat_count, :available_seats, :price)
    )");

    // 初始化随机数种子
    srand((unsigned int)time(nullptr));

    QStringList departureCities = {
        "北京", "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆",
        "武汉", "南京", "青岛", "厦门", "长沙", "郑州", "昆明", "大连",
        "常州", "海南", "苏州", "桂林"
    };
    QStringList arrivalCities = {
        "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆", "武汉",
        "南京", "青岛", "厦门", "三亚", "昆明", "大连", "哈尔滨", "乌鲁木齐","常州", "海南", "苏州", "桂林"
    };

    // 航班前缀列表
    QStringList flightPrefixes = {
        "MU", "CA", "CZ", "FM", "HU",
        "MF", "SC", "3U", "ZH", "HO",
        "9C", "SQ", "TG", "JL", "NH"
    };

    // 循环生成10000条航班数据
    for (int i = 0; i < 10000; ++i) {
        // 生成唯一航班号
        QString prefix = flightPrefixes[rand() % flightPrefixes.size()];
        int num = (i % 999) + 1;
        QString flightNum = QString("%1%2").arg(prefix).arg(num, 3, 10, QChar('0'));

        // 随机出发地和目的地（避免相同）
        int depIdx = rand() % departureCities.size();
        int arrIdx;
        do {
            arrIdx = rand() % arrivalCities.size();
        } while (departureCities[depIdx] == arrivalCities[arrIdx]);
        QString departure = departureCities[depIdx];
        QString destination = arrivalCities[arrIdx];

        // 随机出发时间（当前时间往后1~30天）
        QDateTime departTime = QDateTime::currentDateTime();
        departTime = departTime.addDays(rand() % 30)
                         .addSecs((rand() % 24) * 3600)
                         .addSecs((rand() % 60) * 60);

        // 随机到达时间（出发后1~5小时）
        QDateTime arriveTime = departTime;
        arriveTime = arriveTime.addSecs((1 + rand() % 5) * 3600)
                         .addSecs((rand() % 60) * 60);

        // 随机座位数和票价
        int seatCount = 150 + rand() % 151;
        double price = (int)(600.00 + (rand() % 190001) / 100.0);

        // 绑定参数（包含available_seats，初始等于总座位数）
        query.bindValue(":flight_num", flightNum);
        query.bindValue(":departure", departure);
        query.bindValue(":destination", destination);
        query.bindValue(":depart_time", departTime);
        query.bindValue(":arrive_time", arriveTime);
        query.bindValue(":seat_count", seatCount);
        query.bindValue(":available_seats", seatCount); // 初始可用座位=总座位
        query.bindValue(":price", price);

        // 执行插入
        if (!query.exec()) {
            qWarning() << "插入测试航班失败（序号" << i << "，航班号" << flightNum << "）:" << query.lastError().text();
        }
    }
}

// 初始化数据库表
bool DBManager::initDatabase()
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    // 用户表
    QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INT PRIMARY KEY AUTO_INCREMENT,
            account VARCHAR(50) NOT NULL UNIQUE,
            password VARCHAR(100) NOT NULL,
            role VARCHAR(20) DEFAULT 'user'
        )
    )";
    // 航班表
    QString createFlights = R"(
        CREATE TABLE IF NOT EXISTS flights (
            id INT PRIMARY KEY AUTO_INCREMENT,
            flight_num VARCHAR(20) NOT NULL UNIQUE,
            departure VARCHAR(50) NOT NULL,
            destination VARCHAR(50) NOT NULL,
            depart_time DATETIME NOT NULL,
            arrive_time DATETIME NOT NULL,
            seat_count INT NOT NULL,
            available_seats INT NOT NULL,
            price DECIMAL(10,2) NOT NULL
        )
    )";
    QString createOrders = R"(
    CREATE TABLE IF NOT EXISTS orders (
        id INT PRIMARY KEY AUTO_INCREMENT,
        order_num VARCHAR(50) NOT NULL UNIQUE,
        flight_num VARCHAR(20) NOT NULL,
        departure VARCHAR(50) NOT NULL,
        destination VARCHAR(50) NOT NULL,
        depart_time DATETIME NOT NULL,
        seat_num VARCHAR(10) NOT NULL,
        price DECIMAL(10,2) NOT NULL,
        status VARCHAR(20) NOT NULL DEFAULT '已支付',
        create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (flight_num) REFERENCES flights(flight_num)
    )
)";


    // 执行建表语句
    if(query.exec(createUsers) && query.exec(createFlights)&&query.exec(createOrders)) {
        qDebug() << "数据表检查/创建成功。";
        // 检查并插入测试数据
        if(query.exec("SELECT COUNT(*) FROM flights")&&query.next()){
            int dataCount = query.value(0).toInt();
            if(dataCount < 10000){
                insertTestFlights();
                qDebug()<<"已在表中插入了10000条初始航班数据!";
            }else{
                qDebug()<<"flights 已有10000条数据";
            }
        }else{
            qWarning()<<"查询航班表数据失败"<<query.lastError().text();
        }
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
    if (query.exec("SELECT id, flight_num, departure, destination, depart_time, arrive_time, seat_count, available_seats, price FROM flights ORDER BY depart_time")) {
        while (query.next()) {
            Flight flight;
            flight.setId(query.value("id").toInt());
            flight.setFlightNumber(query.value("flight_num").toString());
            flight.setDepartureCity(query.value("departure").toString());
            flight.setArrivalCity(query.value("destination").toString());
            flight.setDepartureTime(query.value("depart_time").toDateTime());
            flight.setArrivalTime(query.value("arrive_time").toDateTime());
            flight.setTotalSeats(query.value("seat_count").toInt());
            flight.setAvailableSeats(query.value("available_seats").toInt()); // 读取可用座位
            flight.setPrice(query.value("price").toDouble());

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

    QString sql = "SELECT id, flight_num, departure, destination, depart_time, arrive_time, seat_count, available_seats, price FROM flights WHERE 1=1";
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
            flight.setAvailableSeats(query.value("available_seats").toInt()); // 读取可用座位
            flight.setPrice(query.value("price").toDouble());

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
        INSERT INTO flights (flight_num, departure, destination, depart_time, arrive_time, seat_count, available_seats, price)
        VALUES (:flight_num, :departure, :destination, :depart_time, :arrive_time, :seat_count, :available_seats, :price)
    )";

    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":flight_num", flight.flightNumber());
    query.bindValue(":departure", flight.departureCity());
    query.bindValue(":destination", flight.arrivalCity());
    query.bindValue(":depart_time", flight.departureTime());
    query.bindValue(":arrive_time", flight.arrivalTime());
    query.bindValue(":seat_count", flight.totalSeats());
    query.bindValue(":available_seats", flight.availableSeats()); // 插入可用座位
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
        available_seats = :available_seats,
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
    query.bindValue(":available_seats", flight.availableSeats()); // 更新可用座位
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

// 添加用户
bool DBManager::addUser(const QString& account, const QString& password, const QString& role) {
    if (!db.isOpen()) return false;
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (account, password, role) VALUES (:account, :password, :role)");
    query.bindValue(":account", account);
    query.bindValue(":password", password); // 实际项目需加密存储
    query.bindValue(":role", role);
    return query.exec();
}

// 验证用户（双方逻辑一致，保留）
bool DBManager::verifyUser(const QString& account, const QString& password, QString& role) {
    if (!db.isOpen()) return false;
    QSqlQuery query(db);
    query.prepare("SELECT role FROM users WHERE account = :account AND password = :password");
    query.bindValue(":account", account);
    query.bindValue(":password", password);
    if (query.exec() && query.next()) {
        role = query.value("role").toString();
        return true;
    }
    return false;
}
QList<Order> DBManager::getAllOrders()
{
    QList<Order> orders;
    if (!db.isOpen()) return orders;

    QSqlQuery query(db);
    if (query.exec("SELECT id, order_num, flight_num, departure, destination, depart_time, seat_num, price, status FROM orders ORDER BY create_time DESC")) {
        while (query.next()) {
            Order order;
            order.setId(query.value("id").toInt());
            order.setOrderNumber(query.value("order_num").toString());
            order.setFlightNumber(query.value("flight_num").toString());
            order.setDepartureCity(query.value("departure").toString());
            order.setArrivalCity(query.value("destination").toString());
            order.setDepartTime(query.value("depart_time").toDateTime());
            order.setSeatNumber(query.value("seat_num").toString());
            order.setPrice(query.value("price").toDouble());
            order.setStatus(query.value("status").toString());
            orders.append(order);
        }
    } else {
        qWarning() << "查询所有订单失败:" << query.lastError().text();
    }
    return orders;
}

// 2. 筛选订单（航班号/日期/状态）
QList<Order> DBManager::findOrders(const QString& flightNum, const QDate& date, const QString& status)
{
    QList<Order> orders;
    if (!db.isOpen()) return orders;

    QString sql = "SELECT id, order_num, flight_num, departure, destination, depart_time, seat_num, price, status FROM orders WHERE 1=1";
    if (!flightNum.isEmpty()) {
        sql += " AND flight_num LIKE :flightNum";
    }
    if (date.isValid()) {
        sql += " AND DATE(depart_time) = :date";
    }
    if (status != "全部订单") {
        sql += " AND status = :status";
    }
    sql += " ORDER BY create_time DESC";

    QSqlQuery query(db);
    query.prepare(sql);
    if (!flightNum.isEmpty()) {
        query.bindValue(":flightNum", "%" + flightNum + "%");
    }
    if (date.isValid()) {
        query.bindValue(":date", date.toString("yyyy-MM-dd"));
    }
    if (status != "全部订单") {
        query.bindValue(":status", status);
    }

    if (query.exec()) {
        while (query.next()) {
            Order order;
            order.setId(query.value("id").toInt());
            order.setOrderNumber(query.value("order_num").toString());
            order.setFlightNumber(query.value("flight_num").toString());
            order.setDepartureCity(query.value("departure").toString());
            order.setArrivalCity(query.value("destination").toString());
            order.setDepartTime(query.value("depart_time").toDateTime());
            order.setSeatNumber(query.value("seat_num").toString());
            order.setPrice(query.value("price").toDouble());
            order.setStatus(query.value("status").toString());
            orders.append(order);
        }
    } else {
        qWarning() << "筛选订单失败:" << query.lastError().text();
    }
    return orders;
}

bool DBManager::addOrder(const Order& order)
{
    if (!db.isOpen()) return false;

    QString sql = R"(
        INSERT INTO orders (order_num, flight_num, departure, destination, depart_time, seat_num, price, status)
        VALUES (:order_num, :flight_num, :departure, :destination, :depart_time, :seat_num, :price, :status)
    )";

    QSqlQuery query(db);
    query.prepare(sql);
    // 【修复】直接绑定原生类型，取消手动字符串转换（Qt自动适配数据库类型）
    query.bindValue(":order_num", order.orderNumber());
    query.bindValue(":flight_num", order.flightNumber());
    query.bindValue(":departure", order.departureCity());
    query.bindValue(":destination", order.arrivalCity());
    query.bindValue(":depart_time", order.departTime()); // 直接传QDateTime，无需转字符串
    query.bindValue(":seat_num", order.seatNumber());
    query.bindValue(":price", order.price()); // 直接传double，Qt自动适配DECIMAL(10,2)
    query.bindValue(":status", order.status());

    if (query.exec()) {
        qDebug() << "新增订单成功，订单号：" << order.orderNumber();
        return true;
    } else {
        qCritical() << "新增订单失败:" << query.lastError().text(); // 输出具体错误
        return false;
    }
}

// 4. 取消订单（更新状态）
bool DBManager::cancelOrder(int orderId)
{
    if (!db.isOpen() || orderId == 0) return false;

    QString sql = "UPDATE orders SET status = '已取消' WHERE id = :id";
    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":id", orderId);

    if (query.exec()) {
        qDebug() << "取消订单成功。";
        return true;
    } else {
        qWarning() << "取消订单失败:" << query.lastError().text();
        return false;
    }
}

// 5. 根据ID获取订单详情
Order DBManager::getOrderById(int orderId)
{
    Order order;
    if (!db.isOpen() || orderId == 0) return order;

    QSqlQuery query(db);
    query.prepare("SELECT id, order_num, flight_num, departure, destination, depart_time, seat_num, price, status FROM orders WHERE id = :id");
    query.bindValue(":id", orderId);

    if (query.exec() && query.next()) {
        order.setId(query.value("id").toInt());
        order.setOrderNumber(query.value("order_num").toString());
        order.setFlightNumber(query.value("flight_num").toString());
        order.setDepartureCity(query.value("departure").toString());
        order.setArrivalCity(query.value("destination").toString());
        order.setDepartTime(query.value("depart_time").toDateTime());
        order.setSeatNumber(query.value("seat_num").toString());
        order.setPrice(query.value("price").toDouble());
        order.setStatus(query.value("status").toString());
    } else {
        qWarning() << "查询订单详情失败:" << query.lastError().text();
    }
    return order;
}
Flight DBManager::getFlightByFlightNum(const QString& flightNum)
{
    Flight flight;
    if (!db.isOpen()) {
        qWarning() << "数据库未连接，无法查询航班！";
        return flight;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, flight_num, departure, destination, depart_time, arrive_time, seat_count, available_seats, price FROM flights WHERE flight_num = :flight_num");
    query.bindValue(":flight_num", flightNum);

    if (query.exec() && query.next()) {
        flight.setId(query.value("id").toInt());
        flight.setFlightNumber(query.value("flight_num").toString());
        flight.setDepartureCity(query.value("departure").toString());
        flight.setArrivalCity(query.value("destination").toString());
        flight.setDepartureTime(query.value("depart_time").toDateTime());
        flight.setArrivalTime(query.value("arrive_time").toDateTime());
        flight.setTotalSeats(query.value("seat_count").toInt());
        flight.setAvailableSeats(query.value("available_seats").toInt());
        flight.setPrice(query.value("price").toDouble());
    } else {
        qWarning() << "根据航班号查询航班失败:" << query.lastError().text();
    }
    return flight;
}
