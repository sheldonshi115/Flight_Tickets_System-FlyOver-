#ifndef BOARDINGPASS_H
#define BOARDINGPASS_H

#include <QString>
#include <QDateTime>
#include <QPrinter>
#include <QPainter>
#include <QFile>
#include <QTextDocument>
#include "flight.h"

// 登机牌打印类
class BoardingPass
{
public:
    struct PassengerInfo {
        QString name;           // 乘客姓名
        QString idCard;         // 身份证号
        QString seatNumber;     // 座位号
        QString ticketNumber;   // 票号
        QString bookingCode;    // 预订码
    };

    struct FlightInfo {
        QString flightNumber;   // 航班号
        QString departure;      // 出发地
        QString destination;    // 目的地
        QDateTime departTime;   // 出发时间
        QDateTime arriveTime;   // 到达时间
        QString gate;           // 登机口
        QString terminal;       // 航站楼
    };

    // 生成并打印登机牌
    static bool printBoardingPass(const PassengerInfo& passenger, 
                                   const FlightInfo& flight,
                                   const QString& outputPath = "");

    // 生成登机牌HTML
    static QString generateBoardingPassHTML(const PassengerInfo& passenger, 
                                             const FlightInfo& flight);

    // 导出为PDF
    static bool exportToPDF(const QString& html, const QString& outputPath);
};

#endif // BOARDINGPASS_H
