#ifndef ORDER_H
#define ORDER_H

#include <QDateTime>
#include <QString>

class Order
{
public:
    Order() = default;

    // Getter & Setter
    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString orderNumber() const { return m_orderNum; }
    void setOrderNumber(const QString& num) { m_orderNum = num; }

    QString flightNumber() const { return m_flightNum; }
    void setFlightNumber(const QString& num) { m_flightNum = num; }

    QString departureCity() const { return m_departure; }
    void setDepartureCity(const QString& city) { m_departure = city; }

    QString arrivalCity() const { return m_destination; }
    void setArrivalCity(const QString& city) { m_destination = city; }

    QDateTime departTime() const { return m_departTime; }
    void setDepartTime(const QDateTime& time) { m_departTime = time; }

    QString seatNumber() const { return m_seatNum; }
    void setSeatNumber(const QString& num) { m_seatNum = num; }

    double price() const { return m_price; }
    void setPrice(double price) { m_price = price; }

    QString status() const { return m_status; }
    void setStatus(const QString& status) { m_status = status; }

    // 验证订单是否有效

    bool isValid() const {
        return !orderNumber().isEmpty() && !flightNumber().isEmpty() && !seatNumber().isEmpty();
    }

private:
    int m_id = 0;
    QString m_orderNum;    // 订单号（唯一）
    QString m_flightNum;   // 航班号
    QString m_departure;   // 出发城市
    QString m_destination; // 到达城市
    QDateTime m_departTime;// 出发时间
    QString m_seatNum;     // 座位号
    double m_price = 0.0;  // 票价
    QString m_status;      // 订单状态（已支付/已取消）
};

#endif // ORDER_H
