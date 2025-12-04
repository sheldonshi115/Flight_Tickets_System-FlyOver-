#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "flightmanager.h"
#include "ai.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // 新增：设置用户权限（假设管理员为true，普通用户为false）
    void setIsAdmin(bool isAdmin);

private:
    Ui::MainWindow *ui;
    FlightManager *m_flightManager;
    AIQueryWidget *m_aiquery;
    bool m_isAdmin; // 新增：记录当前用户是否为管理员

private slots:
    void onTravelButtonClicked();
    void on_actionFlightManager_triggered();
    void on_btnFlightQuery_clicked();
    void on_btnAIService_clicked();
};
#endif // MAINWINDOW_H
