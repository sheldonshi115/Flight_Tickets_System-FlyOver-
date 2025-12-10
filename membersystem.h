#ifndef MEMBERSYSTEM_H
#define MEMBERSYSTEM_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QSqlDatabase>

// 会员等级枚举
enum class MemberLevel {
    Bronze = 0,     // 青铜会员
    Silver = 1,     // 白银会员
    Gold = 2,       // 黄金会员
    Platinum = 3,   // 铂金会员
    Diamond = 4     // 钻石会员
};

// 会员信息结构
struct MemberInfo {
    QString userId;              // 用户ID
    int points = 0;              // 积分
    double balance = 10000.0;    // 飞机币余额（初始10000元✈）
    double mileage = 0.0;        // 飞行里程（公里）
    MemberLevel level = MemberLevel::Bronze;  // 会员等级
    QDateTime joinDate;          // 加入日期
    
    // 根据里程计算等级
    void updateLevel() {
        if (mileage >= 50000) level = MemberLevel::Diamond;
        else if (mileage >= 30000) level = MemberLevel::Platinum;
        else if (mileage >= 15000) level = MemberLevel::Gold;
        else if (mileage >= 5000) level = MemberLevel::Silver;
        else level = MemberLevel::Bronze;
    }
    
    QString getLevelName() const {
        switch (level) {
            case MemberLevel::Bronze: return "青铜会员";
            case MemberLevel::Silver: return "白银会员";
            case MemberLevel::Gold: return "黄金会员";
            case MemberLevel::Platinum: return "铂金会员";
            case MemberLevel::Diamond: return "钻石会员";
            default: return "普通会员";
        }
    }
    
    QString getLevelIcon() const {
        switch (level) {
            case MemberLevel::Bronze: return "🥉";
            case MemberLevel::Silver: return "🥈";
            case MemberLevel::Gold: return "🥇";
            case MemberLevel::Platinum: return "💎";
            case MemberLevel::Diamond: return "👑";
            default: return "✨";
        }
    }
    
    double getDiscount() const {
        switch (level) {
            case MemberLevel::Bronze: return 1.0;    // 无折扣
            case MemberLevel::Silver: return 0.95;   // 95折
            case MemberLevel::Gold: return 0.90;     // 9折
            case MemberLevel::Platinum: return 0.85; // 85折
            case MemberLevel::Diamond: return 0.80;  // 8折
            default: return 1.0;
        }
    }
};

// 交易记录结构
struct Transaction {
    QString transactionId;
    QString userId;
    QString type;           // "earn"(收入) 或 "spend"(支出)
    double amount;
    QString description;
    QDateTime time;
};

// 代金券结构
struct Voucher {
    QString id;
    QString code;
    double value = 0.0;
    QDateTime expireDate;
    bool used = false;
};

// 会员系统管理类
class MemberSystem : public QObject
{
    Q_OBJECT
    
public:
    static MemberSystem& instance();
    
    // 初始化会员系统（创建数据库表）
    bool initMemberSystem(QSqlDatabase& db);
    
    // 创建新会员（注册时调用）
    bool createMember(const QString& userId);
    
    // 获取会员信息
    MemberInfo getMemberInfo(const QString& userId);
    
    // 更新会员信息
    bool updateMemberInfo(const MemberInfo& info);
    
    // 飞机币操作
    bool addBalance(const QString& userId, double amount, const QString& description);
    bool deductBalance(const QString& userId, double amount, const QString& description);
    
    // 积分操作
    bool addPoints(const QString& userId, int points);
    bool deductPoints(const QString& userId, int points);
    
    // 里程操作
    bool addMileage(const QString& userId, double mileage);
    
    // 获取交易历史
    QList<Transaction> getTransactionHistory(const QString& userId, int limit = 50);
    
    // 计算订单价格（应用会员折扣）
    double calculatePrice(const QString& userId, double originalPrice);

    // 代金券与兑换操作
    // 创建代金券并返回代金券 id（空表示失败）
    QString createVoucher(const QString& userId, double value, int expireDays = 30);

    // 获取可用代金券
    QList<Voucher> getAvailableVouchers(const QString& userId);

    // 将代金券标记为已使用（可传入订单号）
    bool markVoucherUsed(const QString& voucherId, const QString& orderId = QString());

    // 使用积分兑换具体商品或代金券（itemId 可为特定券标识），返回生成的代金券 id（若适用）
    QString redeemPointsForItem(const QString& userId, const QString& itemId, int pointsCost);

private:
    // 确保 members 表中存在会员记录；若不存在则尝试创建一条初始记录
    bool ensureMemberRecord(const QString& userId);

signals:
    void balanceChanged(const QString& userId, double newBalance);
    void pointsChanged(const QString& userId, int newPoints);
    void mileageChanged(const QString& userId, double newMileage);
    void levelUpgraded(const QString& userId, MemberLevel newLevel);
    // Emitted when any member-related field has changed (balance/points/mileage/level)
    void memberInfoChanged(const QString& userId);

private:
    explicit MemberSystem(QObject* parent = nullptr);
    ~MemberSystem() = default;
};

#endif // MEMBERSYSTEM_H
