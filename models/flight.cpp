#include "flight.h"

Flight::Flight()
    : m_price(0.0), m_totalSeats(0), m_availableSeats(0)
{
}

Flight::Flight(const QString& flightNumber,
               const QString& departureCity,
               const QString& arrivalCity,
               const QDateTime& departureTime,
               const QDateTime& arrivalTime,
               double price,
               int totalSeats,
               int availableSeats)
    : m_flightNumber(flightNumber),
    m_departureCity(departureCity),
    m_arrivalCity(arrivalCity),
    m_departureTime(departureTime),
    m_arrivalTime(arrivalTime),
    m_price(price),
    m_totalSeats(totalSeats),
    m_availableSeats(availableSeats)
{
}

bool Flight::increaseSeats(int count)
{
    if (m_availableSeats + count > m_totalSeats)
        return false;
    m_availableSeats += count;
    return true;
}

bool Flight::decreaseSeats(int count)
{
    if (m_availableSeats < count)
        return false;
    m_availableSeats -= count;
    return true;
}
