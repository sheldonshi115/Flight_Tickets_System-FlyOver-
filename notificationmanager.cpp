#include "notificationmanager.h"
#include <QApplication>
#include <QDebug>

NotificationManager& NotificationManager::instance()
{
    static NotificationManager instance;
    return instance;
}

NotificationManager::NotificationManager(QObject* parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_trayMenu(nullptr)
{
    // 初始化定时器 - 每30分钟检查一次提醒
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &NotificationManager::checkReminders);
    m_reminderTimer->start(30 * 60 * 1000); // 30分钟
}

NotificationManager::~NotificationManager()
{
    if (m_trayIcon) {
        delete m_trayIcon;
    }
    if (m_trayMenu) {
        delete m_trayMenu;
    }
}

void NotificationManager::initSystemTray(QWidget* parent)
{
    if (m_trayIcon) {
        return; // 已经初始化
    }

    m_trayIcon = new QSystemTrayIcon(parent);
    m_trayIcon->setIcon(QIcon(":/resources/images/plane_icon.png"));
    m_trayIcon->setToolTip("FlyOver 航班票务系统");

    // 创建托盘菜单
    m_trayMenu = new QMenu(parent);
    
    QAction* showAction = m_trayMenu->addAction("显示主窗口");
    connect(showAction, &QAction::triggered, [parent]() {
        if (parent) {
            parent->show();
            parent->activateWindow();
        }
    });
    
    m_trayMenu->addSeparator();
    
    QAction* quitAction = m_trayMenu->addAction("退出");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    
    // 连接激活信号
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &NotificationManager::onTrayIconActivated);
    
    m_trayIcon->show();
}

void NotificationManager::showNotification(const QString& title, 
                                            const QString& message,
                                            NotificationType type)
{
    // 添加到通知历史
    NotificationItem item;
    item.title = title;
    item.message = message;
    item.type = type;
    item.time = QDateTime::currentDateTime();
    addNotification(item);

    // 系统托盘通知
    if (m_trayIcon && m_trayIcon->isVisible()) {
        QSystemTrayIcon::MessageIcon icon;
        switch (type) {
            case NotificationType::FlightReminder:
                icon = QSystemTrayIcon::Information;
                break;
            case NotificationType::PriceAlert:
                icon = QSystemTrayIcon::Warning;
                break;
            case NotificationType::OrderStatus:
                icon = QSystemTrayIcon::Information;
                break;
            default:
                icon = QSystemTrayIcon::NoIcon;
        }
        
        m_trayIcon->showMessage(title, message, icon, 5000);
    }
}

void NotificationManager::addNotification(const NotificationItem& item)
{
    m_notifications.prepend(item); // 新通知在前
    
    // 只保留最近100条
    if (m_notifications.size() > 100) {
        m_notifications.removeLast();
    }
    
    emit notificationReceived(item);
}

void NotificationManager::markAsRead(int index)
{
    if (index >= 0 && index < m_notifications.size()) {
        m_notifications[index].isRead = true;
    }
}

void NotificationManager::clearNotifications()
{
    m_notifications.clear();
}

void NotificationManager::setFlightReminder(const QString& flightNumber, 
                                             const QDateTime& departTime)
{
    FlightReminder reminder;
    reminder.flightNumber = flightNumber;
    reminder.departTime = departTime;
    m_reminders.append(reminder);
    
    qDebug() << "已设置航班提醒：" << flightNumber << " 起飞时间：" << departTime;
}

void NotificationManager::checkReminders()
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (int i = 0; i < m_reminders.size(); ++i) {
        FlightReminder& reminder = m_reminders[i];
        
        qint64 hoursToDepart = now.secsTo(reminder.departTime) / 3600;
        
        // 24小时提醒
        if (!reminder.reminded24h && hoursToDepart <= 24 && hoursToDepart > 0) {
            showNotification(
                "航班提醒 ✈️",
                QString("您的航班 %1 将在 %2 小时后起飞，请提前做好准备！")
                    .arg(reminder.flightNumber)
                    .arg(hoursToDepart),
                NotificationType::FlightReminder
            );
            reminder.reminded24h = true;
            emit reminderTriggered(reminder.flightNumber);
        }
        
        // 2小时提醒
        if (!reminder.reminded2h && hoursToDepart <= 2 && hoursToDepart > 0) {
            showNotification(
                "紧急提醒 🚨",
                QString("您的航班 %1 将在 %2 小时内起飞，请立即前往机场！")
                    .arg(reminder.flightNumber)
                    .arg(hoursToDepart),
                NotificationType::FlightReminder
            );
            reminder.reminded2h = true;
            emit reminderTriggered(reminder.flightNumber);
        }
        
        // 移除已过期的提醒
        if (hoursToDepart < 0) {
            m_reminders.removeAt(i);
            --i;
        }
    }
}

void NotificationManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        // 双击托盘图标显示主窗口
        QWidget* mainWindow = qobject_cast<QWidget*>(parent());
        if (mainWindow) {
            mainWindow->show();
            mainWindow->activateWindow();
        }
    }
}
