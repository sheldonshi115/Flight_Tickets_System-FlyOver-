#ifndef EMAILREMINDER_H
#define EMAILREMINDER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include "emailsender.h"

// 邮件提醒类型
enum class EmailReminderType {
    TicketBooked,       // 购票成功
    PointsRedeemed,     // 积分兑换成功
    FlightDeparture     // 航班起飞提醒
};

class EmailReminder : public QObject
{
    Q_OBJECT

public:
    static EmailReminder& instance();
    
    // 发送购票成功提醒
    void sendTicketBookedReminder(const QString& userEmail, const QString& userName, 
                                  const QString& flightNumber, const QString& departure, 
                                  const QString& arrival, const QString& departTime,
                                  const QString& seatNumber, double price);
    
    // 发送积分兑换成功提醒
    void sendPointsRedeemedReminder(const QString& userEmail, const QString& userName, 
                                    const QString& itemName, int pointsUsed);
    
    // 发送航班起飞提醒（提醒用户还需配置定时器）
    void sendFlightDepartureReminder(const QString& userEmail, const QString& userName,
                                     const QString& flightNumber, const QString& departure,
                                     const QString& arrival, const QString& departTime);

private:
    explicit EmailReminder(QObject *parent = nullptr);
    ~EmailReminder() = default;
    
    // 禁用复制和赋值
    EmailReminder(const EmailReminder&) = delete;
    EmailReminder& operator=(const EmailReminder&) = delete;
    
    EmailSender m_emailSender;
};

#endif // EMAILREMINDER_H
