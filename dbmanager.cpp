#include "dbmanager.h"
#include <QSqlError>
#include <QDebug>

DbManager::DbManager()
{
}

DbManager::~DbManager()
{
    if (m_db.isOpen())
        m_db.close();
}

DbManager& DbManager::instance()
{
    static DbManager instance;
    return instance;
}

bool DbManager::initDatabase(const QString& dbName)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbName);

    if (!m_db.open()) {
        qDebug() << "Database error:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

bool DbManager::createTables()
{
    QSqlQuery query;
    // 创建航班表
    if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS flights (
            flight_number TEXT PRIMARY KEY,
            departure_city TEXT,
            arrival_city TEXT,
            departure_time TEXT,
            arrival_time TEXT,
            price REAL,
            total_seats INTEGER,
            available_seats INTEGER
        )
    )")) {
        qDebug() << "Table creation error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DbManager::addFlight(const Flight& flight)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO flights (
            flight_number, departure_city, arrival_city,
            departure_time, arrival_time, price,
            total_seats, available_seats
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");

    query.addBindValue(flight.flightNumber());
    query.addBindValue(flight.departureCity());
    query.addBindValue(flight.arrivalCity());
    query.addBindValue(flight.departureTime().toString(Qt::ISODate));
    query.addBindValue(flight.arrivalTime().toString(Qt::ISODate));
    query.addBindValue(flight.price());
    query.addBindValue(flight.totalSeats());
    query.addBindValue(flight.availableSeats());

    if (!query.exec()) {
        qDebug() << "Add flight error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DbManager::updateFlight(const Flight& flight)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE flights SET
            departure_city = ?,
            arrival_city = ?,
            departure_time = ?,
            arrival_time = ?,
            price = ?,
            total_seats = ?,
            available_seats = ?
        WHERE flight_number = ?
    )");

    query.addBindValue(flight.departureCity());
    query.addBindValue(flight.arrivalCity());
    query.addBindValue(flight.departureTime().toString(Qt::ISODate));
    query.addBindValue(flight.arrivalTime().toString(Qt::ISODate));
    query.addBindValue(flight.price());
    query.addBindValue(flight.totalSeats());
    query.addBindValue(flight.availableSeats());
    query.addBindValue(flight.flightNumber());

    if (!query.exec()) {
        qDebug() << "Update flight error:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool DbManager::removeFlight(const QString& flightNumber)
{
    QSqlQuery query;
    query.prepare("DELETE FROM flights WHERE flight_number = ?");
    query.addBindValue(flightNumber);

    if (!query.exec()) {
        qDebug() << "Remove flight error:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

Flight DbManager::getFlight(const QString& flightNumber)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM flights WHERE flight_number = ?");
    query.addBindValue(flightNumber);

    if (!query.exec() || !query.next()) {
        qDebug() << "Get flight error or not found:" << query.lastError().text();
        return Flight();
    }

    return Flight(
        query.value("flight_number").toString(),
        query.value("departure_city").toString(),
        query.value("arrival_city").toString(),
        QDateTime::fromString(query.value("departure_time").toString(), Qt::ISODate),
        QDateTime::fromString(query.value("arrival_time").toString(), Qt::ISODate),
        query.value("price").toDouble(),
        query.value("total_seats").toInt(),
        query.value("available_seats").toInt()
        );
}

QList<Flight> DbManager::getAllFlights()
{
    QList<Flight> flights;
    QSqlQuery query("SELECT * FROM flights ORDER BY departure_time ASC");

    while (query.next()) {
        flights.append(Flight(
            query.value("flight_number").toString(),
            query.value("departure_city").toString(),
            query.value("arrival_city").toString(),
            QDateTime::fromString(query.value("departure_time").toString(), Qt::ISODate),
            QDateTime::fromString(query.value("arrival_time").toString(), Qt::ISODate),
            query.value("price").toDouble(),
            query.value("total_seats").toInt(),
            query.value("available_seats").toInt()
            ));
    }

    if (query.lastError().type() != QSqlError::NoError) {
        qDebug() << "Get all flights error:" << query.lastError().text();
    }
    return flights;
}

QList<Flight> DbManager::findFlights(const QString& departure, const QString& arrival, const QDateTime& date)
{
    QList<Flight> flights;
    QSqlQuery query;

    QString sql = "SELECT * FROM flights WHERE 1=1";
    QList<QVariant> bindValues;

    if (!departure.isEmpty()) {
        sql += " AND departure_city LIKE ?";
        bindValues.append("%" + departure + "%");
    }

    if (!arrival.isEmpty()) {
        sql += " AND arrival_city LIKE ?";
        bindValues.append("%" + arrival + "%");
    }

    if (date.isValid()) {
        QDate targetDate = date.date();
        sql += " AND departure_time >= ? AND departure_time <= ?";
        bindValues.append(targetDate.toString(Qt::ISODate) + "T00:00:00");
        bindValues.append(targetDate.toString(Qt::ISODate) + "T23:59:59");
    }

    sql += " ORDER BY departure_time ASC";

    query.prepare(sql);
    for (int i = 0; i < bindValues.size(); ++i) {
        query.bindValue(i, bindValues[i]);
    }

    if (!query.exec()) {
        qDebug() << "Find flights error:" << query.lastError().text();
        return flights;
    }

    while (query.next()) {
        flights.append(Flight(
            query.value("flight_number").toString(),
            query.value("departure_city").toString(),
            query.value("arrival_city").toString(),
            QDateTime::fromString(query.value("departure_time").toString(), Qt::ISODate),
            QDateTime::fromString(query.value("arrival_time").toString(), Qt::ISODate),
            query.value("price").toDouble(),
            query.value("total_seats").toInt(),
            query.value("available_seats").toInt()
            ));
    }

    return flights;
}
