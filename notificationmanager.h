#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QDateTime>

// 通知类型枚举
enum class NotificationType {
    FlightReminder,      // 航班提醒
    PriceAlert,          // 价格提醒
    OrderStatus,         // 订单状态
    SystemMessage        // 系统消息
};

// 通知项结构
struct NotificationItem {
    QString title;
    QString message;
    NotificationType type;
    QDateTime time;
    bool isRead = false;
};

// 通知管理器类
class NotificationManager : public QObject
{
    Q_OBJECT
    
public:
    static NotificationManager& instance();
    
    // 显示系统托盘通知
    void showNotification(const QString& title, const QString& message, 
                         NotificationType type = NotificationType::SystemMessage);
    
    // 添加通知到历史记录
    void addNotification(const NotificationItem& item);
    
    // 获取通知历史
    QList<NotificationItem> getNotifications() const { return m_notifications; }
    
    // 标记为已读
    void markAsRead(int index);
    
    // 清空通知
    void clearNotifications();
    
    // 设置航班提醒
    void setFlightReminder(const QString& flightNumber, const QDateTime& departTime);
    
    // 初始化系统托盘
    void initSystemTray(QWidget* parent);

signals:
    void notificationReceived(const NotificationItem& item);
    void reminderTriggered(const QString& flightNumber);

private slots:
    void checkReminders();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    explicit NotificationManager(QObject* parent = nullptr);
    ~NotificationManager();
    
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QList<NotificationItem> m_notifications;
    QTimer* m_reminderTimer;
    
    struct FlightReminder {
        QString flightNumber;
        QDateTime departTime;
        bool reminded24h = false;
        bool reminded2h = false;
    };
    QList<FlightReminder> m_reminders;
};

#endif // NOTIFICATIONMANAGER_H
