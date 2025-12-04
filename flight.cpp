// flight.cpp
#include "flight.h"

// 构造函数：初始化所有成员变量
Flight::Flight() :
    m_id(0),
    m_price(0.0),
    m_totalSeats(0),
    m_availableSeats(0)
{}

// --- Getters 实现 ---
int Flight::id() const {
    return m_id;
}
QString Flight::flightNumber() const {
    return m_flightNumber;
}
QString Flight::departureCity() const {
    return m_departureCity;
}
QString Flight::arrivalCity() const {
    return m_arrivalCity;
}
QDateTime Flight::departureTime() const {
    return m_departureTime;
}
QDateTime Flight::arrivalTime() const {
    return m_arrivalTime;
}
double Flight::price() const {
    return m_price;
}
int Flight::totalSeats() const {
    return m_totalSeats;
}
int Flight::availableSeats() const {
    return m_availableSeats;
}

// --- Setters 实现 ---
void Flight::setId(int id) {
    m_id = id;
}
void Flight::setFlightNumber(const QString& number) {
    m_flightNumber = number;
}
void Flight::setDepartureCity(const QString& city) {
    m_departureCity = city;
}
void Flight::setArrivalCity(const QString& city) {
    m_arrivalCity = city;
}
void Flight::setDepartureTime(const QDateTime& dt) {
    m_departureTime = dt;
}
void Flight::setArrivalTime(const QDateTime& dt) {
    m_arrivalTime = dt;
}
void Flight::setPrice(double price) {
    m_price = price;
}
void Flight::setTotalSeats(int seats) {
    m_totalSeats = seats;
}
void Flight::setAvailableSeats(int seats) {
    m_availableSeats = seats;
}
