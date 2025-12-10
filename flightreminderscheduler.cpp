#include "flightreminderscheduler.h"
#include "dbmanager.h"
#include "emailreminder.h"
#include <QDebug>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>

FlightReminderScheduler& FlightReminderScheduler::instance()
{
    static FlightReminderScheduler instance;
    return instance;
}

FlightReminderScheduler::FlightReminderScheduler(QObject *parent)
    : QObject(parent)
{
    // 连接定时器的超时信号
    connect(&m_checkTimer, &QTimer::timeout, this, &FlightReminderScheduler::checkAndSendReminders);
    
    qDebug() << "[FlightReminderScheduler] 初始化完成";
}

void FlightReminderScheduler::startScheduler()
{
    if (m_checkTimer.isActive()) {
        qDebug() << "[FlightReminderScheduler] 定时器已运行，忽略启动请求";
        return;
    }
    
    // 默认每 5 分钟检查一次
    m_checkTimer.start(300000);
    qDebug() << "[FlightReminderScheduler] 定时检查已启动，间隔：300秒";
}

void FlightReminderScheduler::stopScheduler()
{
    if (m_checkTimer.isActive()) {
        m_checkTimer.stop();
        qDebug() << "[FlightReminderScheduler] 定时检查已停止";
    }
}

void FlightReminderScheduler::setCheckInterval(int msec)
{
    m_checkTimer.setInterval(msec);
    qDebug() << "[FlightReminderScheduler] 检查间隔已更新为" << msec << "毫秒";
}

void FlightReminderScheduler::checkAndSendReminders()
{
    qDebug() << "[FlightReminderScheduler] 开始检查航班起飞提醒...";
    
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "[FlightReminderScheduler] 数据库未打开，跳过检查";
        return;
    }
    
    // 查询所有状态为"已支付"且订单对应航班即将起飞的订单
    // 条件：航班起飞时间在当前时间的 30 分钟内，且已超过当前时间
    QDateTime now = QDateTime::currentDateTime();
    QDateTime thirtyMinutesLater = now.addSecs(1800); // 30分钟 = 1800秒
    
    QString sql = R"(
        SELECT o.id, o.order_num, o.flight_num, o.departure, o.destination, 
               o.depart_time, o.account, f.depart_time
        FROM orders o
        JOIN flights f ON o.flight_num = f.flight_num
        WHERE o.status = '已支付'
              AND f.depart_time > :now
              AND f.depart_time <= :thirtyMinutesLater
        ORDER BY f.depart_time ASC
    )";
    
    QSqlQuery query(db);
    query.prepare(sql);
    query.bindValue(":now", now);
    query.bindValue(":thirtyMinutesLater", thirtyMinutesLater);
    
    if (!query.exec()) {
        qWarning() << "[FlightReminderScheduler] 查询订单失败：" << query.lastError().text();
        return;
    }
    
    int reminderCount = 0;
    while (query.next()) {
        QString orderId = query.value("id").toString();
        QString orderNum = query.value("order_num").toString();
        QString flightNumber = query.value("flight_num").toString();
        QString departure = query.value("departure").toString();
        QString destination = query.value("destination").toString();
        QDateTime departTime = query.value("depart_time").toDateTime();
        QString userAccount = query.value("account").toString();
        
        // 检查是否已发送过提醒
        bool alreadySent = false;
        for (const ReminderSentRecord& record : m_sentRecords) {
            if (record.orderId == orderId && record.flightNumber == flightNumber) {
                alreadySent = true;
                break;
            }
        }
        
        if (alreadySent) {
            qDebug() << "[FlightReminderScheduler] 订单" << orderNum << "的提醒已发送过，跳过";
            continue;
        }
        
        // 获取用户邮箱并发送提醒
        if (!userAccount.isEmpty()) {
            UserProfile userProfile = DBManager::instance().loadUserProfile(userAccount);
            if (!userProfile.email.isEmpty()) {
                qDebug() << "[FlightReminderScheduler] 向用户" << userAccount << "发送起飞提醒...";
                
                EmailReminder::instance().sendFlightDepartureReminder(
                    userProfile.email,
                    userProfile.nickname.isEmpty() ? userAccount : userProfile.nickname,
                    flightNumber,
                    departure,
                    destination,
                    departTime.toString("yyyy-MM-dd HH:mm")
                );
                
                // 记录已发送的提醒
                ReminderSentRecord record;
                record.orderId = orderId;
                record.flightNumber = flightNumber;
                record.departTime = departTime;
                m_sentRecords.append(record);
                reminderCount++;
                
                qDebug() << "[FlightReminderScheduler] 成功发送提醒给" << userAccount;
            } else {
                qWarning() << "[FlightReminderScheduler] 用户" << userAccount << "未设置邮箱，跳过发送";
            }
        } else {
            qWarning() << "[FlightReminderScheduler] 订单" << orderNum << "没有关联用户账号";
        }
    }
    
    qDebug() << "[FlightReminderScheduler] 本次检查完成，发送提醒数：" << reminderCount;
    
    // 清理过期的记录（已起飞的航班记录可以删除）
    QDateTime cutoffTime = now.addDays(-1);  // 删除一天前的记录
    for (int i = m_sentRecords.size() - 1; i >= 0; --i) {
        if (m_sentRecords[i].departTime < cutoffTime) {
            m_sentRecords.removeAt(i);
        }
    }
}
