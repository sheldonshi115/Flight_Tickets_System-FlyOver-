#ifndef ANNOUNCEMENTMARQUEE_H
#define ANNOUNCEMENTMARQUEE_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QList>
#include <QPainter>
#include "flight.h"
#include "order.h"

// 公告项数据结构
struct AnnouncementItem {
    enum Type {
        FlightRecommend,    // 航班推荐
        TodayFlight,        // 今日航班
        BoardingReminder,   // 登机提醒
        PrintReminder       // 打印登机牌提醒
    };
    
    Type type;
    QString text;
    QString icon;
    QString flightNumber;
    QDateTime departTime;
    
    AnnouncementItem() : type(FlightRecommend) {}
    AnnouncementItem(Type t, const QString& txt, const QString& ico = "")
        : type(t), text(txt), icon(ico) {}
};

// 公告滚动条组件
class AnnouncementMarquee : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int scrollOffset READ scrollOffset WRITE setScrollOffset)

public:
    explicit AnnouncementMarquee(QWidget *parent = nullptr);
    ~AnnouncementMarquee();

    // 设置当前用户
    void setCurrentUser(const QString& userId);
    
    // 刷新公告内容
    void refreshAnnouncements();
    
    // 开始/停止滚动
    void startScrolling();
    void stopScrolling();
    
    // 滚动偏移量
    int scrollOffset() const { return m_scrollOffset; }
    void setScrollOffset(int offset);

signals:
    void announcementClicked(const AnnouncementItem& item);
    void boardingReminderClicked(const QString& orderNumber);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onScrollTimer();
    void checkUpcomingFlights();

private:
    void generateAnnouncementText();
    void loadTodayFlights();
    void loadUserUpcomingFlights();
    void loadRandomRecommendations();
    QString formatAnnouncementText();
    
    QList<AnnouncementItem> m_announcements;
    QString m_currentUser;
    
    QTimer *m_scrollTimer;
    QTimer *m_refreshTimer;
    int m_scrollOffset;
    int m_textWidth;
    int m_scrollSpeed;
    bool m_isPaused;
    
    QString m_displayText;
    QFont m_font;
};

#endif // ANNOUNCEMENTMARQUEE_H
