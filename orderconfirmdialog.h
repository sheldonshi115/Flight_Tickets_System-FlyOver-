#ifndef ORDERCONFIRMDIALOG_H
#define ORDERCONFIRMDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "flight.h"

class OrderConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    struct OrderInfo {
        QString flightNumber;
        QString departureCity;
        QString arrivalCity;
        QString departureTime;
        QString seatNumber;
        double originalPrice;
        QString voucherCode;
        double voucherValue;
        double finalPrice;
        double userBalance;
    };

    explicit OrderConfirmDialog(const OrderInfo& info, QWidget *parent = nullptr);
    ~OrderConfirmDialog();

private slots:
    void onConfirm();
    void onCancel();

private:
    void setupUI();

    OrderInfo m_info;
};

#endif // ORDERCONFIRMDIALOG_H
