#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QList>
#include <QDateTime>   // 用于订单号生成/时间处理
#include <QUuid>       // 用于生成唯一订单号
#include <QMessageBox> // 购票弹窗提示
#include <QTimer>      // 用于延迟刷新（取消订单信号处理）
#include "flight.h"    // 包含 Flight 数据结构
#include "commondefs.h"// 订单类（需确保项目中有该头文件）

namespace Ui {
class FlightManager;
}

class FlightManager : public QWidget
{
    Q_OBJECT // 必须保留，支持信号槽机制

public:
    explicit FlightManager(QWidget *parent = nullptr);
    ~FlightManager();

    // 公共方法声明
    void setupTableView();                  // 初始化航班表格
    void loadFlightsToTable(const QList<Flight>& flights); // 加载航班数据到表格
    Flight getSelectedFlight();             // 获取选中的航班信息
    void setAdminMode(bool isAdminMode);    // 设置管理员模式（显示/隐藏增删改按钮）

signals:
    void orderCreated(); // 购票成功后通知 OrderManager 刷新订单列表

private slots:
    // 手动关联的业务槽函数
    void onAddFlightClicked();              // 新增航班
    void onEditFlightClicked();             // 编辑航班
    void onSearchFlightsClicked();          // 搜索航班
    void onSelectSeatClicked();             // 选择座位
    void onBookTicketClicked();             // 确认购票
    void onRefreshClicked();                // 刷新航班列表

    // 自动关联的槽函数（Qt 自动识别命名规则）
    void on_twFlightList_itemSelectionChanged(); // 航班表格选中行变化
    void on_btnDelete_clicked();            // 删除航班
    void on_btnExit_clicked();              // 返回主页

private:
    Ui::FlightManager *ui;                  // UI 指针
    bool m_isAdminMode = false;             // 管理员模式标记（C++11 就地初始化）
    QString m_selectedSeat;                 // 选中的座位号（如 "1A"）
    QString m_currentFlightNo;              // 当前选中的航班号
    bool m_isManualClick = true;            // 标记是否为手动点击按钮（修复：移到 private 区域）
    QTimer *m_refreshTimer;                 // 延迟刷新定时器（可选，优化取消订单刷新）

    // 私有工具函数
    void restoreSelectedFlight();           // 恢复选中的航班行（刷新后）
    QString generateOrderNumber();          // 生成唯一订单号（ORD+时间戳+UUID）
};

#endif // FLIGHTMANAGER_H
