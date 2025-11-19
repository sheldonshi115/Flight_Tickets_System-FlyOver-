#ifndef FLIGHT_H
#define FLIGHT_H

#include <QString>
#include <QDateTime>

class Flight
{
public:
    Flight();
    Flight(const QString& flightNumber,
           const QString& departureCity,
           const QString& arrivalCity,
           const QDateTime& departureTime,
           const QDateTime& arrivalTime,
           double price,
           int totalSeats,
           int availableSeats = 0,
           int id = 0);

    // Getters
    QString flightNumber() const { return m_flightNumber; }
    QString departureCity() const { return m_departureCity; }
    QString arrivalCity() const { return m_arrivalCity; }
    QDateTime departureTime() const { return m_departureTime; }
    QDateTime arrivalTime() const { return m_arrivalTime; }
    double price() const { return m_price; }
    int totalSeats() const { return m_totalSeats; }
    int availableSeats() const { return m_availableSeats; }

    // Setters
    void setFlightNumber(const QString& flightNumber) { m_flightNumber = flightNumber; }
    void setDepartureCity(const QString& departureCity) { m_departureCity = departureCity; }
    void setArrivalCity(const QString& arrivalCity) { m_arrivalCity = arrivalCity; }
    void setDepartureTime(const QDateTime& departureTime) { m_departureTime = departureTime; }
    void setArrivalTime(const QDateTime& arrivalTime) { m_arrivalTime = arrivalTime; }
    void setPrice(double price) { m_price = price; }
    void setTotalSeats(int totalSeats) { m_totalSeats = totalSeats; }
    void setAvailableSeats(int availableSeats) { m_availableSeats = availableSeats; }
    void setId(int id) { m_id = id; }
    int id() const { return m_id; }

    // 增减座位数（用于订票/退票）
    bool increaseSeats(int count);
    bool decreaseSeats(int count);

private:
    QString m_flightNumber;        // 航班号
    QString m_departureCity;       // 出发城市
    QString m_arrivalCity;         // 到达城市
    QDateTime m_departureTime;     // 出发时间
    QDateTime m_arrivalTime;       // 到达时间
    double m_price;                // 票价
    int m_totalSeats;              // 总座位数
    int m_availableSeats;          // 可用座位数
    int m_id;                      // id成员
};

#endif // FLIGHT_H
