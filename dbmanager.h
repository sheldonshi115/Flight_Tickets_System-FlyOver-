#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include "models/flight.h"

class DbManager
{
public:
    static DbManager& instance();
    bool initDatabase(const QString& dbName = "flight_db.sqlite");
    bool createTables();

    // 航班管理
    bool addFlight(const Flight& flight);
    bool updateFlight(const Flight& flight);
    bool removeFlight(const QString& flightNumber);
    Flight getFlight(const QString& flightNumber);
    QList<Flight> getAllFlights();
    QList<Flight> findFlights(const QString& departure, const QString& arrival, const QDateTime& date);

private:
    DbManager();
    ~DbManager();
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
