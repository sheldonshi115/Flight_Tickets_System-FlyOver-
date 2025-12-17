#include "dbmanager.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <ctime>
#include <cstdlib>
#include <QStandardPaths>
#include <QFile>

// 使用你的数据库配置
const QString DBManager::DB_NAME = "flight_ticket_db";
const QString DBManager::DB_HOST = "localhost";
const QString DBManager::DB_USER = "root";       // 你的MySQL用户名
const QString DBManager::DB_PWD = "dsy20241431@"; // 你的MySQL密码
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
        db.setDatabaseName(QString("DRIVER={MySQL ODBC 8.0 Unicode Driver};SERVER=%1;DATABASE=%2;UID=%3;PWD=%4;PORT=%5;CHARSET=utf8mb4;")
                               .arg(DB_HOST, DB_NAME, DB_USER, DB_PWD, QString::number(DB_PORT)));

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

    // 循环生成300条航班数据
    for (int i = 0; i < 300; ++i) {
        // 生成唯一航班号
        QString prefix = flightPrefixes[rand() % flightPrefixes.size()];
        int num = (i % 300) + 1;
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
            salt VARCHAR(16) NOT NULL, -- 新增：存储16位盐值
            nickname VARCHAR(50) DEFAULT '',    -- 新增：昵称
            phone VARCHAR(20) UNIQUE,           -- 新增：电话（唯一）
            email VARCHAR(100) UNIQUE,          -- 新增：邮箱（唯一）
            gender VARCHAR(10) DEFAULT '未知',  -- 新增：性别
            image VARCHAR(255) DEFAULT '',     -- 新增：头像路径
            role VARCHAR(20) DEFAULT 'user'     -- 角色（保留原字段）
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
    
    // 座位状态表（新增）
    QString createSeats = R"(
        CREATE TABLE IF NOT EXISTS flight_seats (
            id INT PRIMARY KEY AUTO_INCREMENT,
            flight_num VARCHAR(20) NOT NULL,
            seat_id VARCHAR(10) NOT NULL,
            status VARCHAR(20) DEFAULT 'available',
            UNIQUE KEY unique_seat (flight_num, seat_id),
            FOREIGN KEY (flight_num) REFERENCES flights(flight_num) ON DELETE CASCADE
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
        account VARCHAR(50),
        create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (flight_num) REFERENCES flights(flight_num),
        FOREIGN KEY (account) REFERENCES users(account)
    )
)";
    QString createMoments = R"(
    CREATE TABLE IF NOT EXISTS moments (
        id INT PRIMARY KEY AUTO_INCREMENT,
        content TEXT,
        images TEXT, -- 逗号分隔图片路径
        user_name VARCHAR(50) NOT NULL,
        like_count INT DEFAULT 0,
        comment_count INT DEFAULT 0,
        publish_time DATETIME NOT NULL,
        liked TINYINT DEFAULT 0 -- 0=未点赞，1=已点赞
    )
)";

    QString createComments = R"(
    CREATE TABLE IF NOT EXISTS comments (
        id INT PRIMARY KEY AUTO_INCREMENT,
        moment_id INT NOT NULL,
        content TEXT NOT NULL,
        user_name VARCHAR(50) NOT NULL,
        create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (moment_id) REFERENCES moments(id) ON DELETE CASCADE
    )
)";

    QString createSystemEmails = R"(
        CREATE TABLE IF NOT EXISTS system_emails (
            id INT PRIMARY KEY AUTO_INCREMENT,
            user_account VARCHAR(50) NOT NULL,
            subject VARCHAR(255) NOT NULL,
            body TEXT NOT NULL,
            create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
            is_read TINYINT DEFAULT 0,
            FOREIGN KEY (user_account) REFERENCES users(account) ON DELETE CASCADE
        )
    )";


    // 执行建表语句
    if(query.exec(createUsers) && query.exec(createFlights) && query.exec(createSeats) && query.exec(createOrders)&& query.exec(createMoments) && query.exec(createComments) && query.exec(createSystemEmails)) {
        qDebug() << "数据表检查/创建成功。";
        // 初始化会员系统（members 表与 transactions 表）
        // 使用 MemberSystem 单例来创建必要的会员表和交易记录表
        if (!MemberSystem::instance().initMemberSystem(db)) {
            qWarning() << "初始化会员系统失败：" << db.lastError().text();
        } else {
            qDebug() << "会员系统初始化完成。";
        }

        // 检查并插入测试数据
        if(query.exec("SELECT COUNT(*) FROM flights")&&query.next()){
            int dataCount = query.value(0).toInt();
            if(dataCount < 300){
                insertTestFlights();
                qDebug()<<"已在表中插入了300条初始航班数据!";
            }else{
                qDebug()<<"flights 已有300条数据";
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
        INSERT INTO orders (order_num, flight_num, departure, destination, depart_time, seat_num, price, status, account)
        VALUES (:order_num, :flight_num, :departure, :destination, :depart_time, :seat_num, :price, :status, :account)
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
    query.bindValue(":account", order.userId()); // 新增：保存用户账号

    if (query.exec()) {
        qDebug() << "新增订单成功，订单号：" << order.orderNumber() << "，用户：" << order.userId();
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

// 清空所有订单
bool DBManager::clearAllOrders()
{
    if (!db.isOpen()) return false;
    
    QSqlQuery query(db);
    if (query.exec("DELETE FROM orders")) {
        qDebug() << "所有订单已清空";
        return true;
    } else {
        qWarning() << "清空订单失败:" << query.lastError().text();
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

// 保存用户信息到数据库
bool DBManager::saveUserProfile(const UserProfile& profile)
{
    if (!db.isOpen()) {
        qWarning() << "数据库未连接，无法保存用户信息！";
        return false;
    }

    if (profile.account.isEmpty()) {
        qWarning() << "账号不能为空，无法保存用户信息！";
        return false;
    }

    QSqlQuery query(db);
    // 转换性别为字符串
    QString genderStr;
    switch(profile.gender) {
    case Gender::Male: genderStr = "男"; break;
    case Gender::Female: genderStr = "女"; break;
    default: genderStr = "未知"; break;
    }

    // 获取头像文件路径（如果有的话）
    QString imagePath;
    if (!profile.avatar.isNull()) {
        // 生成头像文件名路径
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        imagePath = appDataPath + "/avatars/" + profile.account + ".png";
    }

    // 处理 phone 和 email - 空字符串转为 NULL，避免 UNIQUE 约束问题
    QVariant phoneValue = profile.phone.trimmed().isEmpty() ? QVariant() : QVariant(profile.phone.trimmed());
    QVariant emailValue = profile.email.trimmed().isEmpty() ? QVariant() : QVariant(profile.email.trimmed());

    qDebug() << "准备保存用户信息:"
             << "account=" << profile.account
             << "nickname=" << profile.nickname
             << "phone=" << profile.phone
             << "email=" << profile.email
             << "gender=" << genderStr
             << "image=" << imagePath;

    // 使用 UPDATE 语句更新用户信息（假设账号不变）
    query.prepare(R"(
        UPDATE users SET nickname = :nickname, phone = :phone, email = :email, 
                        gender = :gender, image = :image
        WHERE account = :account
    )");
    query.bindValue(":nickname", profile.nickname);
    query.bindValue(":phone", phoneValue);
    query.bindValue(":email", emailValue);
    query.bindValue(":gender", genderStr);
    query.bindValue(":image", imagePath);
    query.bindValue(":account", profile.account);

    if (!query.exec()) {
        qWarning() << "保存用户信息失败:" << query.lastError().text();
        qWarning() << "SQL语句:" << query.lastQuery();
        return false;
    }

    int rowsAffected = query.numRowsAffected();
    qDebug() << "用户信息保存成功，受影响行数:" << rowsAffected << "账号:" << profile.account;
    
    if (rowsAffected == 0) {
        qWarning() << "警告：没有任何行被更新，账号可能不存在:" << profile.account;
    }
    
    return true;
}


// 从数据库加载用户信息
UserProfile DBManager::loadUserProfile(const QString& account)
{
    UserProfile profile;
    profile.account = account; // 确保账号被设置
    
    if (!db.isOpen()) {
        qWarning() << "数据库未连接，无法加载用户信息！";
        profile.gender = Gender::Unknown;
        return profile;
    }

    qDebug() << "开始加载用户信息，account=" << account;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT account, nickname, phone, email, gender, image FROM users WHERE account = :account
    )");
    query.bindValue(":account", account);

    if (!query.exec()) {
        qWarning() << "查询用户信息失败:" << query.lastError().text();
        profile.gender = Gender::Unknown;
        return profile;
    }

    if (query.next()) {
        profile.account = query.value("account").toString();
        profile.nickname = query.value("nickname").toString();
        profile.phone = query.value("phone").isNull() ? "" : query.value("phone").toString();
        profile.email = query.value("email").isNull() ? "" : query.value("email").toString();

        // 转换性别字符串为枚举
        QString genderStr = query.value("gender").toString();
        if (genderStr == "男") profile.gender = Gender::Male;
        else if (genderStr == "女") profile.gender = Gender::Female;
        else profile.gender = Gender::Unknown;

        // 加载头像
        QString imagePath = query.value("image").toString();
        if (!imagePath.isEmpty() && QFile::exists(imagePath)) {
            if (!profile.avatar.load(imagePath)) {
                qWarning() << "加载头像失败:" << imagePath;
                qDebug() << "尝试加载的路径:" << imagePath << "存在:" << QFile::exists(imagePath);
            } else {
                qDebug() << "头像加载成功:" << imagePath;
            }
        }

        qDebug() << "用户信息加载成功:" << account 
                 << "nickname:" << profile.nickname 
                 << "phone:" << profile.phone 
                 << "email:" << profile.email
                 << "gender:" << (profile.gender == Gender::Male ? "男" : profile.gender == Gender::Female ? "女" : "未知");
    }

    return profile;
}

// 座位状态管理
bool DBManager::markSeatAsSold(const QString& flightNum, const QString& seatId)
{
    if (!db.isOpen()) return false;
    
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO flight_seats (flight_num, seat_id, status)
        VALUES (:flight_num, :seat_id, 'sold')
        ON DUPLICATE KEY UPDATE status = 'sold'
    )");
    query.bindValue(":flight_num", flightNum);
    query.bindValue(":seat_id", seatId);
    
    if (query.exec()) {
        qDebug() << "座位标记为已售：" << flightNum << seatId;
        return true;
    } else {
        qWarning() << "标记座位失败:" << query.lastError().text();
        return false;
    }
}

bool DBManager::isSeatSold(const QString& flightNum, const QString& seatId)
{
    if (!db.isOpen()) return false;
    
    QSqlQuery query(db);
    query.prepare("SELECT status FROM flight_seats WHERE flight_num = :flight_num AND seat_id = :seat_id");
    query.bindValue(":flight_num", flightNum);
    query.bindValue(":seat_id", seatId);
    
    if (query.exec() && query.next()) {
        return query.value("status").toString() == "sold";
    }
    return false;
}

QStringList DBManager::getSoldSeats(const QString& flightNum)
{
    QStringList soldSeats;
    if (!db.isOpen()) return soldSeats;
    
    QSqlQuery query(db);
    query.prepare("SELECT seat_id FROM flight_seats WHERE flight_num = :flight_num AND status = 'sold'");
    query.bindValue(":flight_num", flightNum);
    
    if (query.exec()) {
        while (query.next()) {
            soldSeats << query.value("seat_id").toString();
        }
    }
    return soldSeats;
}
bool DBManager::addMoment(const MomentItem& item) {
    if (!db.isOpen()) return false;
    QSqlQuery query(db);
    // 拼接图片路径（逗号分隔）
    QString imagesStr = item.images.join(",");
    query.prepare(R"(
        INSERT INTO moments (content, images, user_name, like_count, comment_count, publish_time, liked)
        VALUES (:content, :images, :user_name, :like_count, :comment_count, :publish_time, :liked)
    )");
    query.bindValue(":content", item.content);
    query.bindValue(":images", imagesStr);
    query.bindValue(":user_name", item.userName);
    query.bindValue(":like_count", item.likeCount);
    query.bindValue(":comment_count", item.commentCount);
    query.bindValue(":publish_time", item.publishTime);
    query.bindValue(":liked", item.liked ? 1 : 0);
    return query.exec();
}

// ========== 新增：获取所有动态 ==========


// ========== 新增：更新动态点赞状态 ==========
bool DBManager::updateMomentLike(int momentId, bool liked) {
    if (!db.isOpen()) return false;
    QSqlQuery query(db);
    // 先查询当前点赞数
    query.prepare("SELECT like_count, liked FROM moments WHERE id = :id");
    query.bindValue(":id", momentId);
    if (!query.exec() || !query.next()) return false;

    int currLike = query.value("like_count").toInt();
    bool currLiked = query.value("liked").toInt() == 1;
    int newLike = currLike;
    if (liked && !currLiked) {
        newLike += 1;
    } else if (!liked && currLiked) {
        newLike -= 1;
    }

    // 更新点赞状态和点赞数
    query.prepare(R"(
        UPDATE moments SET liked = :liked, like_count = :like_count
        WHERE id = :id
    )");
    query.bindValue(":liked", liked ? 1 : 0);
    query.bindValue(":like_count", newLike);
    query.bindValue(":id", momentId);
    return query.exec();
}

// ========== 新增：添加评论 ==========
bool DBManager::addComment(int momentId, const QString& content, const QString& userName) {
    if (!db.isOpen()) return false;
    QSqlQuery query(db);
    // 1. 添加评论
    query.prepare(R"(
        INSERT INTO comments (moment_id, content, user_name)
        VALUES (:moment_id, :content, :user_name)
    )");
    query.bindValue(":moment_id", momentId);
    query.bindValue(":content", content);
    query.bindValue(":user_name", userName);
    if (!query.exec()) return false;

    // 2. 更新动态的评论数
    return updateMomentCommentCount(momentId);
}

// ========== 新增：更新动态评论数 ==========
bool DBManager::updateMomentCommentCount(int momentId) {
    if (!db.isOpen()) return false;
    QSqlQuery query(db);
    // 先查询评论总数
    query.prepare("SELECT COUNT(*) FROM comments WHERE moment_id = :id");
    query.bindValue(":id", momentId);
    if (!query.exec() || !query.next()) return false;
    int count = query.value(0).toInt();

    // 更新评论数
    query.prepare("UPDATE moments SET comment_count = :count WHERE id = :id");
    query.bindValue(":count", count);
    query.bindValue(":id", momentId);
    return query.exec();
}

// ========== 新增：获取动态的所有评论 ==========
QList<Comment> DBManager::getCommentsByMomentId(int momentId) {
    QList<Comment> list;
    if (!db.isOpen()) return list;
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, content, user_name, create_time
        FROM comments WHERE moment_id = :id ORDER BY create_time DESC
    )");
    query.bindValue(":id", momentId);
    if (!query.exec()) return list;

    while (query.next()) {
        Comment c;
        c.id = query.value("id").toInt();
        c.momentId = momentId;
        c.content = query.value("content").toString();
        c.userName = query.value("user_name").toString();
        c.createTime = query.value("create_time").toDateTime();
        list.append(c);
    }
    return list;
}
QList<MomentItem> DBManager::getAllMoments() {
    QList<MomentItem> list;
    if (!db.isOpen()) {
        qCritical() << "getAllMoments：数据库未连接！";
        return list;
    }

    QSqlQuery query(db);
    QString sql = "SELECT id, content, images, user_name, like_count, comment_count, publish_time, liked FROM moments ORDER BY publish_time DESC";
    if (!query.exec(sql)) {
        qCritical() << "getAllMoments查询失败：" << query.lastError().text() << "SQL语句：" << sql;
        return list;
    }

    while (query.next()) {
        qDebug() << "成功读取1条动态数据，ID：" << query.value("id").toInt();
        MomentItem item;
        item.id = query.value("id").toInt();
        item.content = query.value("content").toString();
        item.images = query.value("images").toString().split(",", Qt::SkipEmptyParts);
        item.userName = query.value("user_name").toString();
        item.avatarPath = getAvatarByNickname(item.userName);
        item.likeCount = query.value("like_count").toInt();
        item.commentCount = query.value("comment_count").toInt();
        item.publishTime = query.value("publish_time").toDateTime();
        item.liked = query.value("liked").toInt() == 1;
        list.append(item);
    }
    qDebug() << "getAllMoments最终返回" << list.size() << "条动态";
    return list;
}

QString DBManager::getAvatarByNickname(const QString& nickname) {
    if (!db.isOpen() || nickname.trimmed().isEmpty()) {
        return QString();
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT image FROM users WHERE nickname = :nick LIMIT 1
    )");
    query.bindValue(":nick", nickname.trimmed());

    if (!query.exec()) {
        qWarning() << "getAvatarByNickname 查询失败：" << query.lastError().text();
        return QString();
    }

    if (query.next()) {
        return query.value(0).toString();
    }
    return QString();
}

bool DBManager::addSystemEmail(const QString& account, const QString& subject, const QString& body)
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    
    // 使用位置参数绑定，避免编码问题
    query.prepare("INSERT INTO system_emails (user_account, subject, body, create_time, is_read) "
                  "VALUES (?, ?, ?, ?, 0)");
    query.addBindValue(account);
    query.addBindValue(subject);
    query.addBindValue(body);
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (!query.exec()) {
        qWarning() << "addSystemEmail failed:" << query.lastError().text();
        return false;
    }
    return true;
}

QList<SystemEmail> DBManager::getSystemEmails(const QString& account)
{
    QList<SystemEmail> emails;
    if (!db.isOpen()) return emails;

    QSqlQuery query(db);
    QString sql = QString("SELECT id, user_account, subject, body, create_time, is_read "
                  "FROM system_emails WHERE user_account = '%1' ORDER BY create_time DESC").arg(account);

    if (query.exec(sql)) {
        while (query.next()) {
            SystemEmail email;
            email.id = query.value("id").toInt();
            email.userAccount = query.value("user_account").toString();
            email.subject = query.value("subject").toString();
            email.body = query.value("body").toString();
            email.createTime = query.value("create_time").toDateTime();
            email.isRead = query.value("is_read").toBool();
            emails.append(email);
        }
    } else {
        qWarning() << "getSystemEmails failed:" << query.lastError().text();
    }
    return emails;
}

bool DBManager::markSystemEmailAsRead(int emailId)
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    QString sql = QString("UPDATE system_emails SET is_read = 1 WHERE id = %1").arg(emailId);
    return query.exec(sql);
}

bool DBManager::deleteSystemEmail(int emailId)
{
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    QString sql = QString("DELETE FROM system_emails WHERE id = %1").arg(emailId);
    return query.exec(sql);
}




