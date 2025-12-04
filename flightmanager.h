// flightmanager.h
#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QList>
#include "flight.h" // 包含 Flight 数据结构

namespace Ui {
class FlightManager;
}

class FlightManager : public QWidget
{
    Q_OBJECT

public:
    explicit FlightManager(QWidget *parent = nullptr);
    ~FlightManager();

    // 您代码中使用的公共/私有方法声明
    void setupTableView();
    void loadFlightsToTable(const QList<Flight>& flights);
    Flight getSelectedFlight();
    void setAdminMode(bool isAdminMode);

private slots:
    // --- 手动 connect 的槽函数 ---
    void onAddFlightClicked();
    void onBookTicketClicked();
    void onEditFlightClicked();
    void onSearchFlightsClicked();
    void onSelectSeatClicked();
    void onRefreshClicked();

    // --- 自动关联槽函数 (如果存在) ---
    void on_twFlightList_itemSelectionChanged(); // 表格选择变化

    // --- 按钮槽函数 (删除、退出) ---
    void on_btnDelete_clicked();
    void on_btnExit_clicked();

private:
    Ui::FlightManager *ui;
    bool m_isAdminMode = false; // 用于 setAdminMode
};

#endif // FLIGHTMANAGER_H
