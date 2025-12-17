#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QList>
#include <QDateTime>   // 用于订单号生成/时间处理
#include <QUuid>       // 用于生成唯一订单号
#include <QMessageBox> // 购票弹窗提示
#include <QTimer>      // 用于延迟刷新（取消订单信号处理）
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFrame>
#include "flight.h"    // 包含 Flight 数据结构
#include "commondefs.h"// 订单类（需确保项目中有该头文件）
#include "membersystem.h" // MemberInfo 类型

namespace Ui {
class FlightManager;
}

class FlightManager : public QWidget
{
    Q_OBJECT // 必须保留，支持信号槽机制

public:
    explicit FlightManager(QWidget *parent = nullptr);
    ~FlightManager();
    
    // 设置当前用户账号（用于扣除飞机币）
    void setCurrentUser(const QString& account);

    // 公共方法声明
    void setupTableView();                  // 初始化航班表格
    void loadFlightsToTable(const QList<Flight>& flights); // 加载航班数据到表格
    Flight getSelectedFlight();             // 获取选中的航班信息
    void setAdminMode(bool isAdminMode);    // 设置管理员模式（显示/隐藏增删改按钮）
    void setCardViewMode(bool cardMode);    // 设置卡片视图模式（普通用户）

signals:
    void orderCreated(); // 购票成功后通知 OrderManager 刷新订单列表
    void flightCardClicked(const Flight& flight); // 卡片点击信号

private slots:
    // 手动关联的业务槽函数
    void onAddFlightClicked();              // 新增航班
    void onEditFlightClicked();             // 编辑航班
    void onSearchFlightsClicked();          // 搜索航班
    void onSelectSeatClicked();             // 选择座位
    void onBookTicketClicked();             // 确认购票
    void onRefreshClicked();                // 刷新航班列表
    void startBookingProcess();             // 一站式购票流程
    void executeBooking(const Flight& flight, const QString& selectedSeat,
                        const QString& appliedVoucherId, const QString& appliedVoucherCode,
                        double voucherValue, double finalPrice, const MemberInfo& memberInfo);  // 执行购票逻辑

    // 自动关联的槽函数（Qt 自动识别命名规则）
    void on_twFlightList_itemSelectionChanged(); // 航班表格选中行变化
    void on_btnDelete_clicked();            // 删除航班

private:
    Ui::FlightManager *ui;                  // UI 指针
    bool m_isAdminMode = false;             // 管理员模式标记（C++11 就地初始化）
    bool m_isCardViewMode = false;          // 卡片视图模式（普通用户）
    QString m_selectedSeat;                 // 选中的座位号（如 "1A"）
    QString m_currentFlightNo;              // 当前选中的航班号
    QString m_currentUserAccount;           // 当前用户账号
    bool m_isManualClick = true;            // 标记是否为手动点击按钮（修复：移到 private 区域）
    QTimer *m_refreshTimer;                 // 延迟刷新定时器（可选，优化取消订单刷新）
    
    // 卡片视图相关
    QScrollArea *m_cardScrollArea = nullptr;
    QWidget *m_cardContainer = nullptr;
    QList<Flight> m_currentFlights;         // 当前显示的航班列表

    // 私有工具函数
    void restoreSelectedFlight();           // 恢复选中的航班行（刷新后）
    QString generateOrderNumber();          // 生成唯一订单号（ORD+时间戳+UUID）
    void applyModernStyle();                // 应用现代化样式
    void loadFlightsToCards(const QList<Flight>& flights); // 加载航班到卡片视图
    QFrame* createFlightCard(const Flight& flight);        // 创建单个航班卡片
};

#endif // FLIGHTMANAGER_H
