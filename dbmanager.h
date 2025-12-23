#ifndef DBMANAGER_H
#define DBMANAGER_H
#include<order.h>
#include<QSqlError>
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QtMath>
#include "flight.h" // 引入 Flight 类
#include "UserProfile.h" // 引入用户信息结构体
#include "membersystem.h" // 引入会员系统

struct SystemEmail {
    int id;
    QString userAccount;
    QString subject;
    QString body;
    QDateTime createTime;
    bool isRead;
};

struct MomentItem {
    int id = 0;
    QString userName = "匿名";
    QString content;
    QStringList images;
    QString avatarPath; // 新增：头像路径（按昵称查询 users.image）
    QDateTime publishTime;
    int likeCount = 0;
    bool liked = false;
    int commentCount = 0;
};

// 评论结构体
struct Comment {
    int id = 0;
    int momentId = 0;
    QString content;
    QString userName;
    QDateTime createTime;
};

class DBManager : public QObject
{
    Q_OBJECT

private:
    // 单例模式：私有构造函数
    explicit DBManager(QObject *parent = nullptr);
    QSqlDatabase db;

    // 数据库配置参数
    static const QString DB_NAME;
    static const QString DB_HOST;
    static const QString DB_USER;
    static const QString DB_PWD;
    static const int DB_PORT;
    static const int MAX_FLIGHT_RECORDS;
    void insertTestFlights();
    void trimFlightsToLimit(int limit);

public:
    // 单例模式：获取唯一实例
    static DBManager &instance();
     bool addOrder(const Order& order);

    // 初始化数据库（创建表）
    bool initDatabase();

    // 航班操作方法
    QList<Flight> getAllFlights();
    QList<Flight> findFlights(const QString& departure, const QString& arrival, const QDateTime& date);
    QSqlDatabase getDatabase() { return db; }
    bool addFlight(const Flight& flight);
    bool updateFlight(const Flight& flight);
    bool removeFlight(int flightId);

    bool addUser(const QString& account, const QString& password, const QString& role = "user");
    QList<Order> getAllOrders(); // 获取所有订单
    QList<Order> getOrdersByUserId(const QString& userId); // 根据用户ID获取订单
    QList<Order> findOrders(const QString& flightNum, const QDate& date, const QString& status); // 筛选订单
    bool clearAllOrders(); // 清空所有订单
     // 新增订单
    bool cancelOrder(int orderId); // 取消订单（更新状态为"已取消"）
    Order getOrderById(int orderId); // 根据ID获取订单详情
    Flight getFlightByFlightNum(const QString& flightNum);
    
    // 座位状态管理
    bool markSeatAsSold(const QString& flightNum, const QString& seatId); // 标记座位为已售
    bool isSeatSold(const QString& flightNum, const QString& seatId); // 检查座位是否已售
    QStringList getSoldSeats(const QString& flightNum); // 获取某航班所有已售座位
    
    // 新增：用户信息保存和加载方法
    bool saveUserProfile(const UserProfile& profile); // 保存用户信息到数据库
    UserProfile loadUserProfile(const QString& account); // 从数据库加载用户信息


    bool addMoment(const MomentItem& item);
    QList<MomentItem> getAllMoments();
    bool updateMomentLike(int momentId, bool liked);
    bool addComment(int momentId, const QString& content, const QString& userName);
    bool updateMomentCommentCount(int momentId);
    QList<Comment> getCommentsByMomentId(int momentId);
    QString getAvatarByNickname(const QString& nickname); // 新增：根据昵称取头像路径

    // 系统邮件相关
    bool addSystemEmail(const QString& account, const QString& subject, const QString& body);
    QList<SystemEmail> getSystemEmails(const QString& account);
    bool markSystemEmailAsRead(int emailId);
    bool deleteSystemEmail(int emailId); // 新增：删除系统邮件

    // 新增：获取最后一次数据库错误信息（调试用）
    QString lastError() const { return db.lastError().text(); }
};

#endif // DBMANAGER_H
