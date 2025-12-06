// ordermanagement.h
#ifndef ORDERMANAGEMENT_H
#define ORDERMANAGEMENT_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QList> // 明确引入 QList

// ====================================================================

class OrderManagementWidget : public QWidget
{
    Q_OBJECT

public:
    // 构造函数和析构函数
    explicit OrderManagementWidget(QWidget *parent = nullptr);
    ~OrderManagementWidget();

private slots:
    // 槽函数
    void onOrderListItemClicked();
    void onCancelOrder();
    void onRefundOrder();
    void onChangeFilter();
    void loadOrders();

private:
    // 私有方法
    void setupUI();
    void displayOrders();

    // 订单数据结构体
    struct Order {
        QString orderNumber;
        QString flightNumber;
        QString airline;
        QString departure;
        QString arrival;
        QString departTime;
        QString status;  // 已预定、已出票、已取消、已完成
        double price;
        int passengers;
        QString date;
    };

    // UI 控件指针
    QComboBox *m_filterCombo;
    QListWidget *m_orderList;
    QLabel *m_orderDetailLabel;
    QPushButton *m_cancelBtn;
    QPushButton *m_refundBtn;
    QLabel *m_priceLabel;

    // 数据成员
    QList<Order> m_orders;
    int m_selectedOrderIndex;

    // 辅助方法
    QString getStatusColor(const QString& status);
    QString getStatusIcon(const QString& status);
};

#endif // ORDERMANAGEMENT_H
