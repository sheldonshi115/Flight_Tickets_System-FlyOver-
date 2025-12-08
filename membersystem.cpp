#include "membersystem.h"
#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>

MemberSystem& MemberSystem::instance()
{
    static MemberSystem instance;
    return instance;
}

MemberSystem::MemberSystem(QObject* parent)
    : QObject(parent)
{
}

bool MemberSystem::initMemberSystem(QSqlDatabase& db)
{
    if (!db.isOpen()) {
        qWarning() << "数据库未打开";
        return false;
    }

    QSqlQuery query(db);
    
    // 创建会员信息表
    QString createMembersTable = R"(
        CREATE TABLE IF NOT EXISTS members (
            user_id VARCHAR(50) PRIMARY KEY,
            points INT DEFAULT 0,
            balance DECIMAL(10,2) DEFAULT 10000.00,
            mileage DECIMAL(10,2) DEFAULT 0.00,
            level INT DEFAULT 0,
            join_date DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    // 创建交易记录表
    QString createTransactionsTable = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            id VARCHAR(50) PRIMARY KEY,
            user_id VARCHAR(50) NOT NULL,
            type VARCHAR(10) NOT NULL,
            amount DECIMAL(10,2) NOT NULL,
            description VARCHAR(200),
            time DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(account) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createMembersTable)) {
        qWarning() << "创建会员表失败：" << query.lastError().text();
        return false;
    }
    
    if (!query.exec(createTransactionsTable)) {
        qWarning() << "创建交易记录表失败：" << query.lastError().text();
        return false;
    }
    
    qDebug() << "会员系统初始化成功";
    return true;
}

bool MemberSystem::createMember(const QString& userId)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO members (user_id, points, balance, mileage, level, join_date)
        VALUES (:user_id, 0, 10000.00, 0.00, 0, :join_date)
    )");
    
    query.bindValue(":user_id", userId);
    query.bindValue(":join_date", QDateTime::currentDateTime());
    
    if (query.exec()) {
        qDebug() << "创建会员成功：" << userId;
        
        // 记录初始赠送
        addBalance(userId, 10000.0, "新用户注册赠送 🎁");
        
        return true;
    } else {
        qWarning() << "创建会员失败：" << query.lastError().text();
        return false;
    }
}

MemberInfo MemberSystem::getMemberInfo(const QString& userId)
{
    MemberInfo info;
    info.userId = userId;
    
    // 使用 DBManager 的数据库连接
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "getMemberInfo: 数据库未打开";
        return info;
    }
    
    QSqlQuery query(db);
    query.prepare("SELECT * FROM members WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);
    
    if (query.exec() && query.next()) {
        info.points = query.value("points").toInt();
        info.balance = query.value("balance").toDouble();
        info.mileage = query.value("mileage").toDouble();
        info.level = static_cast<MemberLevel>(query.value("level").toInt());
        info.joinDate = query.value("join_date").toDateTime();
        
        // 根据里程更新等级
        info.updateLevel();
    } else {
        qWarning() << "获取会员信息失败：" << query.lastError().text();
    }
    
    return info;
}

bool MemberSystem::updateMemberInfo(const MemberInfo& info)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "updateMemberInfo: 数据库未打开";
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE members 
        SET points = :points, balance = :balance, mileage = :mileage, level = :level
        WHERE user_id = :user_id
    )");
    
    query.bindValue(":points", info.points);
    query.bindValue(":balance", info.balance);
    query.bindValue(":mileage", info.mileage);
    query.bindValue(":level", static_cast<int>(info.level));
    query.bindValue(":user_id", info.userId);
    
    if (query.exec()) {
        return true;
    } else {
        qWarning() << "更新会员信息失败：" << query.lastError().text();
        return false;
    }
}

bool MemberSystem::addBalance(const QString& userId, double amount, const QString& description)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "addBalance: 数据库未打开";
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE members SET balance = balance + :amount WHERE user_id = :user_id
    )");
    query.bindValue(":amount", amount);
    query.bindValue(":user_id", userId);
    
    if (query.exec()) {
        // 记录交易
        QString transId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QSqlQuery transQuery(db);
        transQuery.prepare(R"(
            INSERT INTO transactions (id, user_id, type, amount, description, time)
            VALUES (:id, :user_id, 'earn', :amount, :description, :time)
        )");
        transQuery.bindValue(":id", transId);
        transQuery.bindValue(":user_id", userId);
        transQuery.bindValue(":amount", amount);
        transQuery.bindValue(":description", description);
        transQuery.bindValue(":time", QDateTime::currentDateTime());
        transQuery.exec();
        
        // 获取新余额
        MemberInfo info = getMemberInfo(userId);
        emit balanceChanged(userId, info.balance);
        
        return true;
    }
    
    return false;
}

bool MemberSystem::deductBalance(const QString& userId, double amount, const QString& description)
{
    // 先检查余额
    MemberInfo info = getMemberInfo(userId);
    if (info.balance < amount) {
        qWarning() << "余额不足";
        return false;
    }
    
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "deductBalance: 数据库未打开";
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare(R"(
        UPDATE members SET balance = balance - :amount WHERE user_id = :user_id
    )");
    query.bindValue(":amount", amount);
    query.bindValue(":user_id", userId);
    
    if (query.exec()) {
        // 记录交易
        QString transId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QSqlQuery transQuery(db);
        transQuery.prepare(R"(
            INSERT INTO transactions (id, user_id, type, amount, description, time)
            VALUES (:id, :user_id, 'spend', :amount, :description, :time)
        )");
        transQuery.bindValue(":id", transId);
        transQuery.bindValue(":user_id", userId);
        transQuery.bindValue(":amount", amount);
        transQuery.bindValue(":description", description);
        transQuery.bindValue(":time", QDateTime::currentDateTime());
        transQuery.exec();
        
        info = getMemberInfo(userId);
        emit balanceChanged(userId, info.balance);
        
        return true;
    }
    
    return false;
}

bool MemberSystem::addPoints(const QString& userId, int points)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "addPoints: 数据库未打开";
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("UPDATE members SET points = points + :points WHERE user_id = :user_id");
    query.bindValue(":points", points);
    query.bindValue(":user_id", userId);
    
    if (query.exec()) {
        MemberInfo info = getMemberInfo(userId);
        emit pointsChanged(userId, info.points);
        return true;
    }
    
    return false;
}

bool MemberSystem::deductPoints(const QString& userId, int points)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "deductPoints: 数据库未打开";
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("UPDATE members SET points = points - :points WHERE user_id = :user_id");
    query.bindValue(":points", points);
    query.bindValue(":user_id", userId);
    
    return query.exec();
}

bool MemberSystem::addMileage(const QString& userId, double mileage)
{
    MemberInfo oldInfo = getMemberInfo(userId);
    MemberLevel oldLevel = oldInfo.level;
    
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "addMileage: 数据库未打开";
        return false;
    }
    
    QSqlQuery query(db);
    query.prepare("UPDATE members SET mileage = mileage + :mileage WHERE user_id = :user_id");
    query.bindValue(":mileage", mileage);
    query.bindValue(":user_id", userId);
    
    if (query.exec()) {
        MemberInfo newInfo = getMemberInfo(userId);
        newInfo.updateLevel();
        
        // 如果等级提升，更新数据库并发出信号
        if (newInfo.level != oldLevel) {
            QSqlQuery updateQuery(db);
            updateQuery.prepare("UPDATE members SET level = :level WHERE user_id = :user_id");
            updateQuery.bindValue(":level", static_cast<int>(newInfo.level));
            updateQuery.bindValue(":user_id", userId);
            updateQuery.exec();
            
            emit levelUpgraded(userId, newInfo.level);
        }
        
        emit mileageChanged(userId, newInfo.mileage);
        return true;
    }
    
    return false;
}

QList<Transaction> MemberSystem::getTransactionHistory(const QString& userId, int limit)
{
    QList<Transaction> transactions;
    
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "getTransactionHistory: 数据库未打开";
        return transactions;
    }
    
    QSqlQuery query(db);
    query.prepare(R"(
        SELECT * FROM transactions 
        WHERE user_id = :user_id 
        ORDER BY time DESC 
        LIMIT :limit
    )");
    query.bindValue(":user_id", userId);
    query.bindValue(":limit", limit);
    
    if (query.exec()) {
        while (query.next()) {
            Transaction trans;
            trans.transactionId = query.value("id").toString();
            trans.userId = query.value("user_id").toString();
            trans.type = query.value("type").toString();
            trans.amount = query.value("amount").toDouble();
            trans.description = query.value("description").toString();
            trans.time = query.value("time").toDateTime();
            transactions.append(trans);
        }
    }
    
    return transactions;
}

double MemberSystem::calculatePrice(const QString& userId, double originalPrice)
{
    MemberInfo info = getMemberInfo(userId);
    return originalPrice * info.getDiscount();
}
