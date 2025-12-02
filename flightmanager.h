#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H
#include <QWidget>
#include <QDateTime>
#include "flight.h"
#include "dbmanager.h"

namespace Ui {
class FlightManager;
}

class FlightManager : public QWidget
{
    Q_OBJECT

public:
    explicit FlightManager(QWidget *parent = nullptr);
    ~FlightManager();
    void setAdminMode(bool isAdminMode);

private slots:
    void onAddFlightClicked();       // 新增航班按钮
    void onSearchFlightsClicked();   // 查询按钮
    void onRefreshClicked();         // 重置按钮
    void on_btnDelete_clicked();     // 删除按钮
    void on_exitButton_clicked();    // 退出按钮

private:
    Ui::FlightManager *ui;
    bool m_isAdminMode;
    void setupTableView();           // 初始化表格视图
    void loadFlightsToTable(const QList<Flight>& flights); // 加载数据到表格
    Flight getSelectedFlight();      // 获取选中的航班
    bool deleteFlightFromDB(const QString& flightNumber);  // 从数据库删除航班
};

#endif // FLIGHTMANAGER_H
