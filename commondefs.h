// commondefs.h
#ifndef COMMONDEFS_H
#define COMMONDEFS_H

#include <QString>
#include <QVector>

// 座位状态枚举（全局共享）
enum SeatState {
    Available, // 可选
    Sold,      // 已售
    Selected   // 已选
};

// 座位数据结构体（全局共享）
struct SeatData {
    QString seatId;    // 座位号（如"1A"）
    SeatState state;
    bool isWindow;     // 靠窗
    bool isAisle;      // 靠过道
};

// 航班信息结构体（全局共享）
struct FlightInfo {
    QString flightNumber;
    QString departureCity;
    QString arrivalCity;
    QString dateTime;
    QVector<SeatData> allSeats; // 所有座位数据
};

#endif // COMMONDEFS_H
