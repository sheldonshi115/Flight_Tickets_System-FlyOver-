#include "dbmanager.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <ctime>
#include <cstdlib>
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
void DBManager::insertTestFlights()
{
    if (!db.isOpen()) {
        qWarning() << "数据库未连接，无法插入测试数据！";
        return;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO flights (flight_num, departure, destination, depart_time, arrive_time, seat_count, price)
        VALUES (:flight_num, :departure, :destination, :depart_time, :arrive_time, :seat_count, :price)
    )");

    // 修正1：使用 C++ 标准库的 srand 初始化随机数种子（替换 qsrand）
    srand((unsigned int)time(nullptr)); // 用系统时间做种子，确保每次启动数据不同

    QStringList departureCities = {
        "北京", "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆",
        "武汉", "南京", "青岛", "厦门", "长沙", "郑州", "昆明", "大连",
        "常州", "海南", "苏州", "桂林"
    };
    QStringList arrivalCities = {
        "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆", "武汉",
        "南京", "青岛", "厦门", "三亚", "昆明", "大连", "哈尔滨", "乌鲁木齐","常州", "海南", "苏州", "桂林"
    };

    // 先定义需要的航班前缀列表（可根据需求添加/删除）
    QStringList flightPrefixes = {
        "MU", "CA", "CZ", "FM", "HU",
        "MF", "SC", "3U", "ZH", "HO",
        "9C", "SQ", "TG", "JL", "NH"
    };

    // 循环生成5000条航班数据（可调整循环次数）
    for (int i = 0; i < 10000; ++i) {
        // 1. 多前缀+唯一航班号（前缀随机选择，数字001~999循环）
        QString prefix = flightPrefixes[rand() % flightPrefixes.size()]; // 随机选前缀
        int num = (i % 999) + 1; // 数字部分001~999循环（保证同前缀内唯一，跨前缀可重复数字但整体航班号唯一）
        QString flightNum = QString("%1%2").arg(prefix).arg(num, 3, 10, QChar('0'));

        // 2. 随机出发地和目的地（避免相同）
        int depIdx = rand() % departureCities.size();
        int arrIdx;
        do {
            arrIdx = rand() % arrivalCities.size();
        } while (departureCities[depIdx] == arrivalCities[arrIdx]);
        QString departure = departureCities[depIdx];
        QString destination = arrivalCities[arrIdx];

        // 3. 随机出发时间（当前时间往后1~30天，随机小时/分钟）
        QDateTime departTime = QDateTime::currentDateTime();
        departTime = departTime.addDays(rand() % 30) // 0~29天（即1~30天后，原逻辑不变）
                         .addSecs((rand() % 24) * 3600) // 0~23小时
                         .addSecs((rand() % 60) * 60);  // 0~59分钟

        // 4. 随机到达时间（出发后1~5小时，随机分钟）
        QDateTime arriveTime = departTime;
        arriveTime = arriveTime.addSecs((1 + rand() % 5) * 3600) // 1~5小时
                         .addSecs((rand() % 60) * 60);    // 0~59分钟

        // 5. 随机座位数（150~300座）
        int seatCount = 150 + rand() % 151;

        // 6. 随机票价（600.00~2500.00元，优化计算逻辑更简洁）
        double price = (int)(600.00 + (rand() % 190001) / 100.0); // 190001 → 0~190000，除以100得0.00~1900.00，总600~2500

        // 绑定参数
        query.bindValue(":flight_num", flightNum);
        query.bindValue(":departure", departure);
        query.bindValue(":destination", destination);
        query.bindValue(":depart_time", departTime);
        query.bindValue(":arrive_time", arriveTime);
        query.bindValue(":seat_count", seatCount);
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
    // 1. 先创建基础表
    if(!query.exec(createUsers) || !query.exec(createFlights)) {
        qWarning() << "创建数据表失败:" << query.lastError().text();
        return false;
    }

    // 2. 检查并添加 users 表的 nickname 字段（如果不存在）
    query.exec("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'flight_ticket_db' AND TABLE_NAME = 'users' AND COLUMN_NAME = 'nickname'");
    if (!query.next()) {
        if (!query.exec("ALTER TABLE users ADD COLUMN nickname VARCHAR(50) DEFAULT '' COMMENT '用户昵称'")) {
            qWarning() << "添加 nickname 字段失败:" << query.lastError().text();
        }
    }

    // 3. 检查并添加 users 表的 avatar_path 字段（如果不存在）
    query.exec("SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'flight_ticket_db' AND TABLE_NAME = 'users' AND COLUMN_NAME = 'avatar_path'");
    if (!query.next()) {
        if (!query.exec("ALTER TABLE users ADD COLUMN avatar_path VARCHAR(255) DEFAULT '' COMMENT '头像文件路径'")) {
            qWarning() << "添加 avatar_path 字段失败:" << query.lastError().text();
        }
    }

    // 4. 插入测试航班数据（原有逻辑不变）
    qDebug() << "数据表检查/创建成功。";
    if(query.exec("SELECT COUNT(*) FROM flights")&&query.next()){
        int dataCount = query.value(0).toInt();
        if(dataCount < 10000){
            insertTestFlights();
            qDebug()<<"已在表中插入了10000条初始航班数据!";
        }else{
            qDebug()<<"flights 已有10000条";
        }
    }else{
        qWarning()<<"查询航班表数据失败"<<query.lastError().text();
    }
    return true;
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

// 1. 获取用户个人资料（根据账号查询昵称和头像路径）
QVariantMap DBManager::getUserProfile(const QString& account)
{
    QVariantMap profile;
    if (!db.isOpen()) return profile;

    QSqlQuery query(db);
    query.prepare("SELECT nickname, avatar_path FROM users WHERE account = :account");
    query.bindValue(":account", account);

    if (query.exec() && query.next()) {
        profile["nickname"] = query.value("nickname").toString();
        profile["avatar_path"] = query.value("avatar_path").toString();
    } else {
        qWarning() << "查询用户资料失败:" << query.lastError().text();
        profile["nickname"] = "";
        profile["avatar_path"] = "";
    }
    return profile;
}

// 2. 更新用户个人资料（根据账号更新昵称和头像路径）
bool DBManager::updateUserProfile(const QString& account, const QString& nickname, const QString& avatarPath)
{
    if (!db.isOpen() || account.isEmpty()) return false;

    QSqlQuery query(db);
    query.prepare("UPDATE users SET nickname = :nickname, avatar_path = :avatar_path WHERE account = :account");
    query.bindValue(":account", account);
    query.bindValue(":nickname", nickname);
    query.bindValue(":avatar_path", avatarPath);

    if (query.exec()) {
        qDebug() << "更新用户资料成功（账号：" << account << "）";
        return true;
    } else {
        qWarning() << "更新用户资料失败:" << query.lastError().text();
        return false;
    }
}
