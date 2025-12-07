#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "flightmanager.h"
#include "ai.h"
#include "UserProfile.h"
#include <ordermanager.h>
#include "dataanalyticswidget.h"
#include "languagemanager.h"
#include "notificationmanager.h"
#include "membersystem.h"
#include <QPropertyAnimation>
#include <QEvent>
#include <QLabel>
#include <QComboBox>

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
    DataAnalyticsWidget *m_dataAnalytics = nullptr; // 数据分析页面
    QPushButton *m_floatingAIButton = nullptr;
    QPropertyAnimation *m_floatingAnim = nullptr;
    bool m_floatingVisible = false;
    bool m_buttonHovered = false;
    
    // 新增：语言切换控件
    QLabel *m_planeIconLabel = nullptr;     // 小飞机图标
    QComboBox *m_languageCombo = nullptr;   // 语言切换下拉框
    
    // 主页面控件
    QLabel *m_memberLevelLabel = nullptr;    // 会员等级显示
    QLabel *m_pointsLabel = nullptr;         // 积分显示
    QLabel *m_balanceLabel = nullptr;        // 飞机币余额
    QLabel *m_mileageLabel = nullptr;        // 飞行里程
    
    // 浮动按钮控制方法
    void slideOutFloatingButton();
    void slideInFloatingButton();
    void applyTheme(bool isDark); // 应用主题
    void navigateToDefaultPage(); // RBAC: 根据角色导航到默认页面
    
    // 新增：初始化语言切换和通知系统
    void initLanguageSwitch();
    void initNotificationSystem();
    void updateUILanguage();
    
    // 主页面相关方法
    void initHomePage();
    void updateMemberInfo();
    void createQuickActionButtons();
    void createStatisticsCards();

    // 事件过滤器用于检测鼠标靠近左侧
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void on_btnOrders_clicked();
    void onTravelButtonClicked();
    void on_actionFlightManager_triggered();
    void on_btnFlightQuery_clicked();
    void on_btnAIService_clicked();
    void on_btnOrderManager_clicked();
    void on_btnDataAnalytics_clicked(); // 数据分析按钮
    void on_actionLogout_triggered();
    void on_actionAbout_triggered();
    void clicked_btnProfile(); // 会员中心（原我的信息）
public slots:
    void set_account(QString acc);
    void setUserProfile(const UserProfile& profile); // 新增：设置用户信息
};
#endif // MAINWINDOW_H
