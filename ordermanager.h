#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <QWidget>
#include <QList>
#include <QTimer> // 新增：用于延迟恢复按钮状态
#include "order.h"

namespace Ui {
class OrderManager;
}

class OrderManager : public QWidget
{
    Q_OBJECT

public:
    explicit OrderManager(QWidget *parent = nullptr);
    ~OrderManager();
    // 新增：公共刷新接口（购票成功后调用，刷新订单列表）
    void refreshOrderList();
    // 在 OrderManager 类中添加信号声明
signals:
    void orderCanceled(const QString& flightNum); // 取消订单后通知刷新航班

private slots:
    void setupTableView(); // 初始化表格
    void loadOrdersToTable(const QList<Order>& orders); // 加载订单到表格
    Order getSelectedOrder(); // 获取选中的订单
    void on_btnQuery_clicked(); // 查询订单
    void on_btnReset_clicked(); // 重置筛选条件
    void on_btnCancelOrder_clicked(); // 取消订单
    void on_btnViewDetail_clicked(); // 查看订单详情
    void on_btnExit_clicked(); // 返回主页
    void on_twOrderList_itemSelectionChanged(); // 表格选择变化

private:
    Ui::OrderManager *ui;
    bool m_isCancelManual = true; // 新增：标记取消订单是否为手动点击
    // 移除冗余的generateOrderNumber（订单号生成在FlightManager中统一处理）
};

#endif // ORDERMANAGER_H
