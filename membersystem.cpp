#include "membersystem.h"
#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>
#include <QRandomGenerator>
#include "notificationmanager.h"

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
    QString createMembersTable = R"sql(
        CREATE TABLE IF NOT EXISTS members (
            user_id VARCHAR(50) PRIMARY KEY,
            points INT DEFAULT 0,
            balance DECIMAL(10,2) DEFAULT 10000.00,
            mileage DECIMAL(10,2) DEFAULT 0.00,
            level INT DEFAULT 0,
            join_date DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )sql";

    // 创建交易记录表（type 扩展到 32 字符）
    QString createTransactionsTable = R"sql(
        CREATE TABLE IF NOT EXISTS transactions (
            id VARCHAR(50) PRIMARY KEY,
            user_id VARCHAR(50) NOT NULL,
            type VARCHAR(32) NOT NULL,
            amount DECIMAL(10,2) NOT NULL,
            description VARCHAR(200),
            time DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(account) ON DELETE CASCADE
        )
    )sql";

    // 创建代金券表（兑换后生成，可用于订单抵扣）
    QString createVouchersTable = R"sql(
        CREATE TABLE IF NOT EXISTS vouchers (
            id VARCHAR(50) PRIMARY KEY,
            user_id VARCHAR(50) NOT NULL,
            code VARCHAR(100) NOT NULL UNIQUE,
            value DECIMAL(10,2) NOT NULL,
            expire_date DATETIME,
            used TINYINT DEFAULT 0,
            used_time DATETIME,
            used_in_order VARCHAR(50),
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(account) ON DELETE CASCADE
        )
    )sql";

    // 创建兑换/使用记录表（记录用户用积分兑换与使用代金券行为）
    QString createRedemptionsTable = R"sql(
        CREATE TABLE IF NOT EXISTS redemptions (
            id VARCHAR(50) PRIMARY KEY,
            user_id VARCHAR(50) NOT NULL,
            voucher_id VARCHAR(50),
            item_id VARCHAR(50),
            points_used INT DEFAULT 0,
            type VARCHAR(32) DEFAULT 'redeem', -- 'redeem' or 'use'
            related_order VARCHAR(50),
            time DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (user_id) REFERENCES users(account) ON DELETE CASCADE,
            FOREIGN KEY (voucher_id) REFERENCES vouchers(id) ON DELETE SET NULL
        )
    )sql";

    if (!query.exec(createMembersTable)) {
        qWarning() << "创建会员表失败：" << query.lastError().text();
        return false;
    }

    if (!query.exec(createTransactionsTable)) {
        qWarning() << "创建交易记录表失败：" << query.lastError().text();
        return false;
    }

    if (!query.exec(createVouchersTable)) {
        qWarning() << "创建代金券表失败：" << query.lastError().text();
        return false;
    }

    if (!query.exec(createRedemptionsTable)) {
        qWarning() << "创建兑换记录表失败：" << query.lastError().text();
        return false;
    }

    qDebug() << "会员系统初始化成功";
    return true;
}

bool MemberSystem::createMember(const QString& userId)
{
    QSqlQuery query;
    query.prepare(R"sql(
        INSERT INTO members (user_id, points, balance, mileage, level, join_date)
        VALUES (:user_id, 0, 10000.00, 0.00, 0, :join_date)
    )sql");

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
    query.prepare(R"sql(
        UPDATE members
        SET points = :points, balance = :balance, mileage = :mileage, level = :level
        WHERE user_id = :user_id
    )sql");

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
    // 确保会员记录存在（兼容旧用户）
    ensureMemberRecord(userId);

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "addBalance: 数据库未打开";
        return false;
    }

    QSqlQuery query(db);
    query.prepare(R"sql(
        UPDATE members SET balance = balance + :amount WHERE user_id = :user_id
    )sql");
    query.bindValue(":amount", amount);
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        // 记录交易
        QString transId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QSqlQuery transQuery(db);
        transQuery.prepare(R"sql(
            INSERT INTO transactions (id, user_id, type, amount, description, time)
            VALUES (:id, :user_id, 'earn', :amount, :description, :time)
        )sql");
        transQuery.bindValue(":id", transId);
        transQuery.bindValue(":user_id", userId);
        transQuery.bindValue(":amount", amount);
        transQuery.bindValue(":description", description);
        transQuery.bindValue(":time", QDateTime::currentDateTime());
        transQuery.exec();

        // 获取新余额
        MemberInfo info = getMemberInfo(userId);
        emit balanceChanged(userId, info.balance);
        // 聚合信号，通知界面统一刷新（避免重复刷新顺序问题）
        emit memberInfoChanged(userId);

        return true;
    }

    return false;
}

bool MemberSystem::deductBalance(const QString& userId, double amount, const QString& description)
{
    // 先检查余额
    // 确保会员记录存在（兼容旧用户）
    ensureMemberRecord(userId);

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
    query.prepare(R"sql(
        UPDATE members SET balance = balance - :amount WHERE user_id = :user_id
    )sql");
    query.bindValue(":amount", amount);
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        // 记录交易
        QString transId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QSqlQuery transQuery(db);
        transQuery.prepare(R"sql(
            INSERT INTO transactions (id, user_id, type, amount, description, time)
            VALUES (:id, :user_id, 'spend', :amount, :description, :time)
        )sql");
        transQuery.bindValue(":id", transId);
        transQuery.bindValue(":user_id", userId);
        transQuery.bindValue(":amount", amount);
        transQuery.bindValue(":description", description);
        transQuery.bindValue(":time", QDateTime::currentDateTime());
        transQuery.exec();

        info = getMemberInfo(userId);
        emit balanceChanged(userId, info.balance);
        emit memberInfoChanged(userId);

        return true;
    }

    return false;
}

bool MemberSystem::addPoints(const QString& userId, int points)
{
    // 确保会员记录存在（兼容旧用户）
    ensureMemberRecord(userId);

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
        emit memberInfoChanged(userId);
        return true;
    }

    return false;
}

bool MemberSystem::deductPoints(const QString& userId, int points)
{
    // 确保会员记录存在（兼容旧用户）
    ensureMemberRecord(userId);

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "deductPoints: 数据库未打开";
        return false;
    }

    // 先检查是否有足够积分
    MemberInfo info = getMemberInfo(userId);
    if (info.points < points) {
        qWarning() << "deductPoints: 积分不足";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE members SET points = points - :points WHERE user_id = :user_id");
    query.bindValue(":points", points);
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        // 记录交易（使用 transactions 表，type 使用 'points_spend'）
        QString transId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        QSqlQuery transQuery(db);
        transQuery.prepare(R"sql(
            INSERT INTO transactions (id, user_id, type, amount, description, time)
            VALUES (:id, :user_id, 'points_spend', :amount, :description, :time)
        )sql");
        transQuery.bindValue(":id", transId);
        transQuery.bindValue(":user_id", userId);
        transQuery.bindValue(":amount", points);
        transQuery.bindValue(":description", QString("积分兑换，扣除 %1 分").arg(points));
        transQuery.bindValue(":time", QDateTime::currentDateTime());
        transQuery.exec();

        MemberInfo newInfo = getMemberInfo(userId);
        emit pointsChanged(userId, newInfo.points);
        emit memberInfoChanged(userId);
        return true;
    }

    return false;
}

bool MemberSystem::addMileage(const QString& userId, double mileage)
{
    // 确保会员记录存在（兼容旧用户）
    ensureMemberRecord(userId);

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
        // 聚合信号
        emit memberInfoChanged(userId);
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
    query.prepare(R"sql(
        SELECT * FROM transactions
        WHERE user_id = :user_id
        ORDER BY time DESC
        LIMIT :limit
    )sql");
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

QString MemberSystem::createVoucher(const QString& userId, double value, int expireDays)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "createVoucher: 数据库未打开";
        return QString();
    }

    QString vid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString code = QString("VCH-%1").arg(vid.left(8));

    QSqlQuery query(db);
    query.prepare(R"sql(
        INSERT INTO vouchers (id, user_id, code, value, expire_date, used, create_time)
        VALUES (:id, :user_id, :code, :value, :expire_date, 0, :create_time)
    )sql");
    query.bindValue(":id", vid);
    query.bindValue(":user_id", userId);
    query.bindValue(":code", code);
    query.bindValue(":value", value);
    if (expireDays > 0) query.bindValue(":expire_date", QDateTime::currentDateTime().addDays(expireDays));
    else query.bindValue(":expire_date", QVariant());
    query.bindValue(":create_time", QDateTime::currentDateTime());

    if (!query.exec()) {
        qWarning() << "createVoucher 插入失败:" << query.lastError().text();
        return QString();
    }

    // 记录兑换/生成记录
    QSqlQuery red(db);
    QString rid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    red.prepare(R"sql(
        INSERT INTO redemptions (id, user_id, voucher_id, item_id, points_used, type, related_order, time)
        VALUES (:id, :user_id, :voucher_id, NULL, 0, 'redeem', NULL, :time)
    )sql");
    red.bindValue(":id", rid);
    red.bindValue(":user_id", userId);
    red.bindValue(":voucher_id", vid);
    red.bindValue(":time", QDateTime::currentDateTime());
    red.exec();

    return vid;
}

QList<Voucher> MemberSystem::getAvailableVouchers(const QString& userId)
{
    QList<Voucher> list;
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "getAvailableVouchers: 数据库未打开";
        return list;
    }

    QSqlQuery query(db);
    query.prepare(R"sql(
        SELECT id, code, value, expire_date, used FROM vouchers
        WHERE user_id = :user_id AND used = 0 AND (expire_date IS NULL OR expire_date > NOW())
        ORDER BY value DESC
    )sql");
    query.bindValue(":user_id", userId);

    if (query.exec()) {
        while (query.next()) {
            Voucher v;
            v.id = query.value("id").toString();
            v.code = query.value("code").toString();
            v.value = query.value("value").toDouble();
            v.expireDate = query.value("expire_date").toDateTime();
            v.used = query.value("used").toInt() != 0;
            list.append(v);
        }
    } else {
        qWarning() << "getAvailableVouchers 查询失败:" << query.lastError().text();
    }

    return list;
}

bool MemberSystem::markVoucherUsed(const QString& voucherId, const QString& orderId)
{
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "markVoucherUsed: 数据库未打开";
        return false;
    }

    QSqlQuery q(db);
    q.prepare(R"sql(
        UPDATE vouchers SET used = 1, used_time = :used_time, used_in_order = :order WHERE id = :id
    )sql");
    q.bindValue(":used_time", QDateTime::currentDateTime());
    q.bindValue(":order", orderId);
    q.bindValue(":id", voucherId);

    if (!q.exec()) {
        qWarning() << "markVoucherUsed 更新失败:" << q.lastError().text();
        return false;
    }

    // 写入使用记录
    QSqlQuery red(db);
    QString rid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    red.prepare(R"sql(
        INSERT INTO redemptions (id, user_id, voucher_id, item_id, points_used, type, related_order, time)
        SELECT :rid, user_id, id, NULL, 0, 'use', :order, :time FROM vouchers WHERE id = :id
    )sql");
    red.bindValue(":rid", rid);
    red.bindValue(":order", orderId);
    red.bindValue(":time", QDateTime::currentDateTime());
    red.bindValue(":id", voucherId);
    red.exec();

    return true;
}

QString MemberSystem::redeemPointsForItem(const QString& userId, const QString& itemId, int pointsCost)
{
    // 先扣除积分
    if (!deductPoints(userId, pointsCost)) {
        qWarning() << "redeemPointsForItem: 扣积分失败";
        return QString();
    }

    // 根据 itemId 决定行为：如果是预定义代金券则创建代金券，否则仅写入兑换记录
    double voucherValue = 0.0;
    QString createdVoucherId;
    QString recordedItemId = itemId; // 存入 redemptions 的 item_id 字段
    QString giftDesc;

    // 兼容前端使用的 id（PointsShopDialog 可能使用 itm1/itm2/itm3）
    if (itemId == "voucher_5" || itemId == "itm1") voucherValue = 5.0, recordedItemId = "voucher_5";
    else if (itemId == "voucher_10" || itemId == "itm2") voucherValue = 10.0, recordedItemId = "voucher_10";
    else if (itemId == "itm3" || itemId == "random_gift") {
        // 随机礼品：从备选列表中随机选一项，并通过通知告知用户
        QStringList gifts = { QString::fromUtf8("主题行李牌"), QString::fromUtf8("航旅水杯"), QString::fromUtf8("折叠充电线"), QString::fromUtf8("数字会员券(下次加赠)") };
        int idx = 0;
        if (!gifts.isEmpty()) idx = QRandomGenerator::global()->bounded(gifts.size());
        giftDesc = gifts.at(idx);
        recordedItemId = QString("gift:%1").arg(giftDesc.left(40));
    }

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "redeemPointsForItem: 数据库未打开";
        return QString();
    }

    if (voucherValue > 0.0) {
        // 创建代金券
        createdVoucherId = createVoucher(userId, voucherValue, 30);
    }

    // 写入兑换记录（若没有创建代金券，也记录 itemId）
    QSqlQuery red(db);
    QString rid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    red.prepare(R"sql(
        INSERT INTO redemptions (id, user_id, voucher_id, item_id, points_used, type, related_order, time)
        VALUES (:id, :user_id, :voucher_id, :item_id, :points_used, 'redeem', NULL, :time)
    )sql");
    red.bindValue(":id", rid);
    red.bindValue(":user_id", userId);
    if (!createdVoucherId.isEmpty()) red.bindValue(":voucher_id", createdVoucherId);
    else red.bindValue(":voucher_id", QVariant());
    red.bindValue(":item_id", recordedItemId);
    red.bindValue(":points_used", pointsCost);
    red.bindValue(":time", QDateTime::currentDateTime());
    red.exec();

    // 如果是随机礼品，发送通知说明礼品内容；同时通过返回值告知调用者礼品内容（格式 GIFT:...）
    if (!giftDesc.isEmpty()) {
        // 发送应用内通知
        NotificationManager::instance().showNotification(QString::fromUtf8("兑换成功"),
                                                        QString::fromUtf8("您获得了礼品：%1").arg(giftDesc),
                                                        NotificationType::SystemMessage);
        return QString("GIFT:%1").arg(giftDesc);
    }

    return createdVoucherId;
}

bool MemberSystem::ensureMemberRecord(const QString& userId)
{
    if (userId.isEmpty()) return false;

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        qWarning() << "ensureMemberRecord: 数据库未打开";
        return false;
    }

    QSqlQuery q(db);
    q.prepare("SELECT 1 FROM members WHERE user_id = :user_id LIMIT 1");
    q.bindValue(":user_id", userId);
    if (q.exec() && q.next()) {
        return true; // 已存在
    }

    qDebug() << "ensureMemberRecord: 会员记录不存在，直接插入初始记录为" << userId;

    // 直接插入一条成员记录，避免调用 createMember() 以免触发递归（createMember 会调用 addBalance）
    QSqlQuery ins(db);
    ins.prepare(R"sql(
        INSERT INTO members (user_id, points, balance, mileage, level, join_date)
        VALUES (:user_id, 0, 10000.00, 0.00, 0, :join_date)
    )sql");
    ins.bindValue(":user_id", userId);
    ins.bindValue(":join_date", QDateTime::currentDateTime());
    if (!ins.exec()) {
        qWarning() << "ensureMemberRecord: 插入成员记录失败:" << ins.lastError().text();
        return false;
    }

    return true;
}
