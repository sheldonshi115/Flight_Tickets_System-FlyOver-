// ordermanagement.cpp
#include "ordermanagement.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QColor>
#include <QSize>

OrderManagementWidget::OrderManagementWidget(QWidget *parent)
    : QWidget(parent), m_selectedOrderIndex(-1)
{
    setupUI();
    loadOrders();
}

OrderManagementWidget::~OrderManagementWidget()
{
}

void OrderManagementWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // 标题
    QLabel *titleLabel = new QLabel("🧾 订单管理");
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #333; margin-bottom: 8px;");
    mainLayout->addWidget(titleLabel);

    // 筛选栏
    QHBoxLayout *filterLayout = new QHBoxLayout();
    QLabel *filterLabel = new QLabel("筛选");
    filterLabel->setStyleSheet("font-weight: bold; color: #555;");
    m_filterCombo = new QComboBox();
    m_filterCombo->addItems({"全部订单", "待支付", "已预定", "已出票", "已取消", "已完成"});
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(m_filterCombo, 1);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &OrderManagementWidget::onChangeFilter);

    // 订单列表
    m_orderList = new QListWidget();
    m_orderList->setSpacing(8);
    mainLayout->addWidget(m_orderList, 1);

    connect(m_orderList, &QListWidget::itemClicked, this, &OrderManagementWidget::onOrderListItemClicked);

    // 订单详情
    QLabel *detailTitleLabel = new QLabel("订单详情");
    detailTitleLabel->setStyleSheet("font-weight: bold; color: #555; margin-top: 12px;");
    mainLayout->addWidget(detailTitleLabel);

    m_orderDetailLabel = new QLabel("选择订单查看详情");
    m_orderDetailLabel->setStyleSheet(
        "background-color: #fff; "
        "padding: 16px; "
        "border-radius: 6px; "
        "border-left: 4px solid #ff6b6b; "
        "min-height: 100px; "
        "color: #666;"
    );
    m_orderDetailLabel->setWordWrap(true);
    mainLayout->addWidget(m_orderDetailLabel);

    // 价格显示和按钮
    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_priceLabel = new QLabel("总价：¥0");
    m_priceLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #ff6b6b;");
    actionLayout->addWidget(m_priceLabel);
    actionLayout->addStretch();

    m_cancelBtn = new QPushButton("❌ 取消订单");
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setEnabled(false);
    m_refundBtn = new QPushButton("💰 申请退款");
    m_refundBtn->setCursor(Qt::PointingHandCursor);
    m_refundBtn->setEnabled(false);

    actionLayout->addWidget(m_cancelBtn);
    actionLayout->addWidget(m_refundBtn);
    mainLayout->addLayout(actionLayout);

    connect(m_cancelBtn, &QPushButton::clicked, this, &OrderManagementWidget::onCancelOrder);
    connect(m_refundBtn, &QPushButton::clicked, this, &OrderManagementWidget::onRefundOrder);
}

void OrderManagementWidget::loadOrders()
{
    m_orders = {
        {"NO20251001", "CA101", "国航", "广州", "上海", "2025-12-06 21:35", "已出票", 674, 1, "2025-12-06"},
        {"NO20251002", "MU5672", "东航", "广州", "上海", "2025-12-07 18:45", "已预定", 1045, 2, "2025-12-07"},
        {"NO20251003", "CZ3832", "南航", "广州", "上海", "2025-12-08 22:15", "待支付", 980, 1, "2025-12-08"},
        {"NO20251004", "HO1852", "吉祥", "广州", "上海", "2025-12-09 21:25", "已完成", 815, 3, "2025-12-09"},
        {"NO20251005", "CA1866", "国航", "广州", "上海", "2025-12-10 21:40", "已取消", 746, 1, "2025-12-10"},
    };

    displayOrders();
}

void OrderManagementWidget::displayOrders()
{
    m_orderList->clear();

    for (int i = 0; i < m_orders.size(); ++i) {
        const Order& order = m_orders[i];

        QString statusIcon = getStatusIcon(order.status);
        QString statusColor = getStatusColor(order.status);

        QString itemText = QString(
            "%1 <b>%2</b>\n"
            "  %3 | %4 → %5\n"
            "  %6 | %7人 | <span style='color: %8;'><b>%9</b></span>"
        ).arg(statusIcon, order.orderNumber,
              order.flightNumber, order.departure, order.arrival,
              order.date, QString::number(order.passengers),
              statusColor, order.status);

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        item->setSizeHint(QSize(0, 90));
        item->setBackground(QColor(255, 255, 255));

        m_orderList->addItem(item);
    }
}

void OrderManagementWidget::onOrderListItemClicked()
{
    QListWidgetItem *item = m_orderList->currentItem();
    if (item) {
        m_selectedOrderIndex = item->data(Qt::UserRole).toInt();
        const Order& order = m_orders[m_selectedOrderIndex];

        QString detailText = QString(
            "<table style='width: 100%;'>"
            "<tr><td><b>订单号</b></td><td>%1</td></tr>"
            "<tr><td><b>航班号</b></td><td>%2 (%3)</td></tr>"
            "<tr><td><b>出发</b></td><td>%4 <span style='color: #ff9500;'>%5</span></td></tr>"
            "<tr><td><b>到达</b></td><td>%6</td></tr>"
            "<tr><td><b>乘客数</b></td><td>%7人</td></tr>"
            "<tr><td><b>订单状态</b></td><td><span style='color: %8;'><b>%9</b></span></td></tr>"
            "<tr><td><b>价格</b></td><td style='color: #ff6b6b;'>¥%10</td></tr>"
            "</table>"
        ).arg(order.orderNumber, order.flightNumber, order.airline,
              order.departure, order.departTime,
              order.arrival, QString::number(order.passengers),
              getStatusColor(order.status), order.status,
              QString::number((int)order.price));

        m_orderDetailLabel->setText(detailText);
        m_priceLabel->setText(QString("总价：¥%1").arg((int)order.price));

        // 根据状态启用/禁用按钮
        bool canCancel = (order.status == "已预定" || order.status == "待支付");
        bool canRefund = (order.status == "已取消" || order.status == "已完成");
        m_cancelBtn->setEnabled(canCancel);
        m_refundBtn->setEnabled(canRefund);
    }
}

void OrderManagementWidget::onCancelOrder()
{
    if (m_selectedOrderIndex >= 0) {
        m_orders[m_selectedOrderIndex].status = "已取消";
        displayOrders();
    }
}

void OrderManagementWidget::onRefundOrder()
{
    if (m_selectedOrderIndex >= 0) {
        // 处理退款逻辑
    }
}

void OrderManagementWidget::onChangeFilter()
{
    // 根据筛选条件过滤订单
    displayOrders();
}

QString OrderManagementWidget::getStatusColor(const QString& status)
{
    if (status == "已预定") return "#ff9500";
    if (status == "已出票") return "#4caf50";
    if (status == "待支付") return "#f44336";
    if (status == "已取消") return "#999999";
    if (status == "已完成") return "#4caf50";
    return "#333333";
}

QString OrderManagementWidget::getStatusIcon(const QString& status)
{
    if (status == "已预定") return "✈️";
    if (status == "已出票") return "✅";
    if (status == "待支付") return "💳";
    if (status == "已取消") return "❌";
    if (status == "已完成") return "🎉";
    return "📋";
}
