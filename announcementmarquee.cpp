#include "announcementmarquee.h"
#include "dbmanager.h"
#include <QPainter>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>

AnnouncementMarquee::AnnouncementMarquee(QWidget *parent)
    : QWidget(parent)
    , m_scrollOffset(0)
    , m_textWidth(0)
    , m_scrollSpeed(2)
    , m_isPaused(false)
{
    setFixedHeight(40);
    setMinimumWidth(200);
    setCursor(Qt::PointingHandCursor);
    
    // 设置字体
    m_font.setFamily("Microsoft YaHei UI");
    m_font.setPointSize(11);
    m_font.setBold(false);
    
    // 滚动定时器
    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setInterval(30); // ~33fps
    connect(m_scrollTimer, &QTimer::timeout, this, &AnnouncementMarquee::onScrollTimer);
    
    // 刷新定时器（每5分钟刷新一次公告内容）
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(300000); // 5分钟
    connect(m_refreshTimer, &QTimer::timeout, this, &AnnouncementMarquee::refreshAnnouncements);
    m_refreshTimer->start();
    
    // 初始化公告
    refreshAnnouncements();
    startScrolling();
    
    // 检查即将起飞的航班（每分钟检查一次）
    QTimer *checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &AnnouncementMarquee::checkUpcomingFlights);
    checkTimer->start(60000); // 1分钟
}

AnnouncementMarquee::~AnnouncementMarquee()
{
    stopScrolling();
}

void AnnouncementMarquee::setCurrentUser(const QString& userId)
{
    m_currentUser = userId;
    refreshAnnouncements();
}

void AnnouncementMarquee::refreshAnnouncements()
{
    m_announcements.clear();
    
    // 加载今日航班
    loadTodayFlights();
    
    // 加载用户即将起飞的航班提醒
    loadUserUpcomingFlights();
    
    // 加载随机航班推荐
    loadRandomRecommendations();
    
    // 生成显示文本
    generateAnnouncementText();
}

void AnnouncementMarquee::loadTodayFlights()
{
    QList<Flight> allFlights = DBManager::instance().getAllFlights();
    QDate today = QDate::currentDate();
    
    int todayCount = 0;
    for (const Flight& flight : allFlights) {
        if (flight.departureTime().date() == today && todayCount < 3) {
            AnnouncementItem item;
            item.type = AnnouncementItem::TodayFlight;
            item.icon = "✈️";
            item.flightNumber = flight.flightNumber();
            item.departTime = flight.departureTime();
            item.text = QString("【今日航班】%1 %2→%3 %4起飞")
                .arg(flight.flightNumber())
                .arg(flight.departureCity())
                .arg(flight.arrivalCity())
                .arg(flight.departureTime().toString("HH:mm"));
            m_announcements.append(item);
            todayCount++;
        }
    }
}

void AnnouncementMarquee::loadUserUpcomingFlights()
{
    if (m_currentUser.isEmpty()) return;
    
    QList<Order> userOrders = DBManager::instance().getOrdersByUserId(m_currentUser);
    QDateTime now = QDateTime::currentDateTime();
    
    for (const Order& order : userOrders) {
        if (order.status() != "已支付") continue;
        
        QDateTime departTime = order.departTime();
        qint64 minutesUntil = now.secsTo(departTime) / 60;
        
        // 2小时内起飞的航班提醒打印登机牌
        if (minutesUntil > 0 && minutesUntil <= 120) {
            AnnouncementItem item;
            item.type = AnnouncementItem::PrintReminder;
            item.icon = "🎫";
            item.flightNumber = order.flightNumber();
            item.departTime = departTime;
            item.text = QString("【请打印登机牌】您的航班 %1 将于 %2 起飞，请尽快打印登机牌！")
                .arg(order.flightNumber())
                .arg(departTime.toString("HH:mm"));
            m_announcements.prepend(item); // 优先显示
        }
        // 30分钟内起飞的航班提醒登机
        else if (minutesUntil > 0 && minutesUntil <= 30) {
            AnnouncementItem item;
            item.type = AnnouncementItem::BoardingReminder;
            item.icon = "🚨";
            item.flightNumber = order.flightNumber();
            item.departTime = departTime;
            item.text = QString("【紧急提醒】您的航班 %1 即将起飞，请立即前往登机口！")
                .arg(order.flightNumber());
            m_announcements.prepend(item); // 最优先显示
        }
    }
}

void AnnouncementMarquee::loadRandomRecommendations()
{
    QList<Flight> allFlights = DBManager::instance().getAllFlights();
    if (allFlights.isEmpty()) return;
    
    // 随机选择3个航班推荐
    QList<int> selectedIndices;
    int maxRecommend = qMin(3, allFlights.size());
    
    while (selectedIndices.size() < maxRecommend) {
        int idx = QRandomGenerator::global()->bounded(allFlights.size());
        if (!selectedIndices.contains(idx)) {
            selectedIndices.append(idx);
        }
    }
    
    for (int idx : selectedIndices) {
        const Flight& flight = allFlights[idx];
        AnnouncementItem item;
        item.type = AnnouncementItem::FlightRecommend;
        item.icon = "🌟";
        item.flightNumber = flight.flightNumber();
        item.text = QString("【热门推荐】%1 %2→%3 ¥%.0f起")
            .arg(flight.flightNumber())
            .arg(flight.departureCity())
            .arg(flight.arrivalCity())
            .arg(flight.price());
        m_announcements.append(item);
    }
}

void AnnouncementMarquee::generateAnnouncementText()
{
    if (m_announcements.isEmpty()) {
        m_displayText = "    ✈️ 欢迎使用 FlyOver 航班票务系统！祝您旅途愉快！    ";
    } else {
        QStringList texts;
        for (const AnnouncementItem& item : m_announcements) {
            texts.append(QString("    %1 %2    ").arg(item.icon, item.text));
        }
        m_displayText = texts.join("  │  ");
    }
    
    // 计算文本宽度
    QFontMetrics fm(m_font);
    m_textWidth = fm.horizontalAdvance(m_displayText);
    
    // 重置滚动位置
    m_scrollOffset = width();
    
    update();
}

void AnnouncementMarquee::startScrolling()
{
    m_scrollTimer->start();
}

void AnnouncementMarquee::stopScrolling()
{
    m_scrollTimer->stop();
}

void AnnouncementMarquee::setScrollOffset(int offset)
{
    m_scrollOffset = offset;
    update();
}

void AnnouncementMarquee::onScrollTimer()
{
    if (m_isPaused) return;
    
    m_scrollOffset -= m_scrollSpeed;
    
    // 当文本完全滚出左侧后，重置到右侧
    if (m_scrollOffset < -m_textWidth) {
        m_scrollOffset = width();
    }
    
    update();
}

void AnnouncementMarquee::checkUpcomingFlights()
{
    // 定期刷新以检查即将起飞的航班
    loadUserUpcomingFlights();
    generateAnnouncementText();
}

void AnnouncementMarquee::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    
    // 绘制背景渐变
    QLinearGradient bgGradient(0, 0, width(), 0);
    bgGradient.setColorAt(0, QColor(59, 130, 246, 15));   // 淡蓝色
    bgGradient.setColorAt(0.5, QColor(99, 102, 241, 20)); // 淡紫色
    bgGradient.setColorAt(1, QColor(59, 130, 246, 15));   // 淡蓝色
    painter.fillRect(rect(), bgGradient);
    
    // 绘制底部边框
    painter.setPen(QPen(QColor(59, 130, 246, 60), 1));
    painter.drawLine(0, height() - 1, width(), height() - 1);
    
    // 绘制左侧标签
    QString label = "📢 公告";
    QFont labelFont = m_font;
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.setPen(QColor(30, 64, 175));
    QRect labelRect(10, 0, 70, height());
    painter.drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, label);
    
    // 绘制分隔线
    painter.setPen(QPen(QColor(59, 130, 246, 80), 1));
    painter.drawLine(85, 8, 85, height() - 8);
    
    // 设置滚动区域裁剪
    QRect scrollArea(95, 0, width() - 100, height());
    painter.setClipRect(scrollArea);
    
    // 绘制滚动文本
    painter.setFont(m_font);
    
    // 渐变文本颜色
    QLinearGradient textGradient(m_scrollOffset, 0, m_scrollOffset + m_textWidth, 0);
    textGradient.setColorAt(0, QColor(30, 64, 175));
    textGradient.setColorAt(0.5, QColor(99, 102, 241));
    textGradient.setColorAt(1, QColor(30, 64, 175));
    
    painter.setPen(QPen(QBrush(textGradient), 1));
    painter.drawText(m_scrollOffset, 0, m_textWidth, height(), 
                     Qt::AlignVCenter | Qt::AlignLeft, m_displayText);
}

void AnnouncementMarquee::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // 点击时可以跳转到相关页面
    emit announcementClicked(m_announcements.isEmpty() ? AnnouncementItem() : m_announcements.first());
}

void AnnouncementMarquee::enterEvent(QEnterEvent *event)
{
    Q_UNUSED(event);
    m_isPaused = true; // 鼠标悬停时暂停滚动
}

void AnnouncementMarquee::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_isPaused = false; // 鼠标离开时继续滚动
}

void AnnouncementMarquee::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // 重新计算文本位置
    if (m_scrollOffset > width()) {
        m_scrollOffset = width();
    }
}
