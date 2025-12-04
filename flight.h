// flight.h (Revised - 仅包含声明)
#ifndef FLIGHT_H
#define FLIGHT_H

#include <QString>
#include <QDateTime>

class Flight
{
public:
    // 构造函数
    Flight();

    // Getters
    int id() const;
    QString flightNumber() const;
    QString departureCity() const;
    QString arrivalCity() const;
    QDateTime departureTime() const;
    QDateTime arrivalTime() const;
    double price() const;
    int totalSeats() const;
    int availableSeats() const;

    // Setters
    void setId(int id);
    void setFlightNumber(const QString& number);
    void setDepartureCity(const QString& city);
    void setArrivalCity(const QString& city);
    void setDepartureTime(const QDateTime& dt);
    void setArrivalTime(const QDateTime& dt);
    void setPrice(double price);
    void setTotalSeats(int seats);
    void setAvailableSeats(int seats);

    bool isValid() const {
        // 逻辑：id>0或航班号非空即视为有效
        return m_id > 0 || !m_flightNumber.isEmpty();
    }

private:
    // 成员变量
    int m_id;
    QString m_flightNumber;
    QString m_departureCity;
    QString m_arrivalCity;
    QDateTime m_departureTime;
    QDateTime m_arrivalTime;
    double m_price;
    int m_totalSeats;
    int m_availableSeats;
};

#endif // FLIGHT_H
