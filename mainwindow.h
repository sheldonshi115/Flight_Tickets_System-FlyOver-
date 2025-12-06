#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "flightmanager.h"
#include "ai.h"
#include "UserProfile.h"
#include<ordermanager.h>
#include <QPropertyAnimation>
#include<QEvent>
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
    bool m_isAdmin;
    UserProfile m_appUser;
    OrderManager *m_orderManager = nullptr;// 新增：记录当前用户是否为管理员
    QPushButton *m_floatingAIButton = nullptr;
    QPropertyAnimation *m_floatingAnim = nullptr;
    bool m_floatingVisible = false;
    bool m_buttonHovered = false;
    // 浮动按钮控制方法
    void slideOutFloatingButton();
    void slideInFloatingButton();

    // 事件过滤器用于检测鼠标靠近左侧
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void on_btnOrders_clicked();
    void onTravelButtonClicked();
    void on_actionFlightManager_triggered();
    void on_btnFlightQuery_clicked();
    void on_btnAIService_clicked();
    void on_btnOrderManager_clicked();
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void clicked_btnProfile();
public slots:
    void set_account(QString acc);
};
#endif // MAINWINDOW_H
