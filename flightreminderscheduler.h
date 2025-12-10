#ifndef FLIGHTREMINDERSCHEDULER_H
#define FLIGHTREMINDERSCHEDULER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QList>
#include <QDateTime>
// 用于追踪已发送的提醒，避免重复发送
struct ReminderSentRecord {
    QString orderId;
    QString flightNumber;
    QDateTime departTime;
};

class FlightReminderScheduler : public QObject
{
    Q_OBJECT

public:
    static FlightReminderScheduler& instance();
    
    // 启动定时检查（在主窗口初始化时调用）
    void startScheduler();
    
    // 停止定时检查
    void stopScheduler();
    
    // 设置检查间隔（毫秒），默认300秒（5分钟）
    void setCheckInterval(int msec);

private slots:
    // 定时检查并发送提醒
    void checkAndSendReminders();

private:
    explicit FlightReminderScheduler(QObject *parent = nullptr);
    ~FlightReminderScheduler() = default;
    
    // 禁用复制和赋值
    FlightReminderScheduler(const FlightReminderScheduler&) = delete;
    FlightReminderScheduler& operator=(const FlightReminderScheduler&) = delete;
    
    QTimer m_checkTimer;
    QList<ReminderSentRecord> m_sentRecords;  // 记录已发送的提醒
};

#endif // FLIGHTREMINDERSCHEDULER_H
