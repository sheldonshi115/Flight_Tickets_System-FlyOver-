#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "flightmanager.h"
#include "ai.h"
#include "ticketbooking.h"
#include "ordermanagement.h"
#include "dataanalyticswidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setIsAdmin(bool isAdmin);
    void applyTheme(bool isDarkMode);

private:
    Ui::MainWindow *ui;
    FlightManager *m_flightManager;
    AIQueryWidget *m_aiquery;
    TicketBookingWidget *m_ticketBooking;
    OrderManagementWidget *m_orderManagement;
    DataAnalyticsWidget *m_dataAnalytics;
    bool m_isAdmin;
    bool m_isDarkMode;

private slots:
    void onTravelButtonClicked();
    void on_actionFlightManager_triggered();
    void on_btnFlightQuery_clicked();
    void on_btnAIService_clicked();
    void on_btnTicketBooking_clicked();
    void on_btnOrderManagement_clicked();
    void on_btnDataAnalytics_clicked();
    void toggleTheme();
};
#endif // MAINWINDOW_H
