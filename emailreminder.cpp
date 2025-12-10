#include "emailreminder.h"
#include "emailconfig.h"
#include <QDebug>
#include <QDateTime>

EmailReminder& EmailReminder::instance()
{
    static EmailReminder instance;
    return instance;
}

EmailReminder::EmailReminder(QObject *parent)
    : QObject(parent)
{
    // 配置 SMTP 服务器
    m_emailSender.setSMTPConfig(
        EmailConfig::SMTP_SERVER,
        EmailConfig::SMTP_PORT,
        EmailConfig::FROM_EMAIL,
        EmailConfig::AUTH_CODE,
        EmailConfig::USE_SSL
        );
}

void EmailReminder::sendTicketBookedReminder(const QString& userEmail, const QString& userName,
                                             const QString& flightNumber, const QString& departure,
                                             const QString& arrival, const QString& departTime,
                                             const QString& seatNumber, double price)
{
    if (userEmail.isEmpty()) {
        qWarning() << "[EmailReminder] 用户邮箱为空，无法发送购票提醒";
        return;
    }

    QString subject = QString("【飞越订票】购票成功提醒 - %1").arg(flightNumber);

    // 修复：将 double 类型的 price 转为保留2位小数的 QString，匹配 arg 占位符
    QString body = QString(
                       "尊敬的 %1 用户，\n\n"
                       "感谢您的购票！您的购票信息如下：\n\n"
                       "━━━━━━━━━━━━━━━━━━━━━━━━\n"
                       "航班号：%2\n"
                       "出发城市：%3\n"
                       "到达城市：%4\n"
                       "出发时间：%5\n"
                       "座位号：%6\n"
                       "票价：￥%7\n"  // 占位符改为 %7，匹配后续 arg 参数数量
                       "━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
                       "请妥善保管您的登机牌，祝您旅途愉快！\n\n"
                       "此邮件由飞越订票系统自动发送，请勿回复。\n"
                       "如有疑问，请联系客服：service@flyover.com"
                       ).arg(userName, flightNumber, departure, arrival, departTime, seatNumber,
                            QString::number(price, 'f', 2));  // 核心修复：double 转 QString（保留2位小数）

    if (m_emailSender.sendEmail(userEmail, subject, body)) {
        qDebug() << "[EmailReminder] 购票成功提醒已发送给" << userEmail;
    } else {
        qWarning() << "[EmailReminder] 购票成功提醒发送失败，收件人：" << userEmail;
    }
}

void EmailReminder::sendPointsRedeemedReminder(const QString& userEmail, const QString& userName,
                                               const QString& itemName, int pointsUsed)
{
    if (userEmail.isEmpty()) {
        qWarning() << "[EmailReminder] 用户邮箱为空，无法发送积分兑换提醒";
        return;
    }

    QString subject = QString("【飞越订票】积分兑换成功提醒");

    QString body = QString(
                       "尊敬的 %1 用户，\n\n"
                       "您的积分兑换申请已成功处理！\n\n"
                       "━━━━━━━━━━━━━━━━━━━━━━━━\n"
                       "兑换项目：%2\n"
                       "消耗积分：%3 分\n"
                       "兑换时间：%4\n"
                       "━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
                       "感谢您对飞越订票的支持！\n"
                       "您可以登录会员中心查看您的兑换记录。\n\n"
                       "此邮件由飞越订票系统自动发送，请勿回复。\n"
                       "如有疑问，请联系客服：service@flyover.com"
                       ).arg(userName, itemName, QString::number(pointsUsed), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    if (m_emailSender.sendEmail(userEmail, subject, body)) {
        qDebug() << "[EmailReminder] 积分兑换提醒已发送给" << userEmail;
    } else {
        qWarning() << "[EmailReminder] 积分兑换提醒发送失败，收件人：" << userEmail;
    }
}

void EmailReminder::sendFlightDepartureReminder(const QString& userEmail, const QString& userName,
                                                const QString& flightNumber, const QString& departure,
                                                const QString& arrival, const QString& departTime)
{
    if (userEmail.isEmpty()) {
        qWarning() << "[EmailReminder] 用户邮箱为空，无法发送航班起飞提醒";
        return;
    }

    QString subject = QString("【飞越订票】航班起飞提醒 - %1").arg(flightNumber);

    QString body = QString(
                       "尊敬的 %1 用户，\n\n"
                       "您的航班即将在 30 分钟内起飞，请准时出发！\n\n"
                       "━━━━━━━━━━━━━━━━━━━━━━━━\n"
                       "航班号：%2\n"
                       "出发城市：%3\n"
                       "到达城市：%4\n"
                       "起飞时间：%5\n"
                       "━━━━━━━━━━━━━━━━━━━━━━━━\n\n"
                       "温馨提示：\n"
                       "• 请提前 2 小时到达机场\n"
                       "• 携带有效身份证和机票\n"
                       "• 遵守机场安全规定\n\n"
                       "祝您旅途安全愉快！\n\n"
                       "此邮件由飞越订票系统自动发送，请勿回复。\n"
                       "如有疑问，请联系客服：service@flyover.com"
                       ).arg(userName, flightNumber, departure, arrival, departTime);

    if (m_emailSender.sendEmail(userEmail, subject, body)) {
        qDebug() << "[EmailReminder] 航班起飞提醒已发送给" << userEmail;
    } else {
        qWarning() << "[EmailReminder] 航班起飞提醒发送失败，收件人：" << userEmail;
    }
}
