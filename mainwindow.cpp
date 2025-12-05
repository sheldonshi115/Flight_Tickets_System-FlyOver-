#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "views/travelmoment.h"
#include <QPropertyAnimation>
#include <QEasingCurve>
#include "ai.h"
#include<ordermanager.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_flightManager(nullptr),
    m_aiquery(nullptr),
    m_isAdmin(false),
    m_orderManager(nullptr)
{
    ui->setupUi(this);

    // 原有信号槽（保留）
    connect(ui->btnHome, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageHome);
    });
    connect(ui->travelbutton, &QPushButton::clicked, this, &MainWindow::onTravelButtonClicked);
    connect(ui->btnFlightQuery, &QPushButton::clicked, this, &MainWindow::on_btnFlightQuery_clicked);
    connect(ui->btnAIService, &QPushButton::clicked, this, &MainWindow::on_btnAIService_clicked);
    connect(ui->btnOrders, &QPushButton::clicked, this, &MainWindow::on_btnOrders_clicked);
    connect(ui->actionViewOrders, &QAction::triggered, this, &MainWindow::on_btnOrders_clicked);

    // 【新增】提前初始化OrderManager（确保信号连接时已存在）
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
    }

    setWindowTitle("航班票务系统 - 主菜单");
    setIsAdmin(true);
}

void MainWindow::on_btnAIService_clicked(){
    AIQueryWidget *aiq = new AIQueryWidget(nullptr);
    aiq->show();
}
MainWindow::~MainWindow()
{
    delete ui;
    // 无需手动删除m_flightManager（父对象为stackedWidget，会自动释放）
}

// 实现权限设置函数
void MainWindow::setIsAdmin(bool isAdmin)
{
    m_isAdmin = isAdmin;
}

void MainWindow::onTravelButtonClicked()
{
    // 原有旅行动态逻辑（保留）
    for (int i = 0; i < ui->stackedWidget->count(); ++i) {
        QWidget *w = ui->stackedWidget->widget(i);
        TravelMoment *tm = qobject_cast<TravelMoment*>(w);
        if (tm) {
            ui->stackedWidget->setCurrentWidget(tm);
            return;
        }
    }
    TravelMoment *tm = new TravelMoment(ui->stackedWidget); // 父对象设为stackedWidget
    tm->setObjectName("travelMomentWidget");
    ui->stackedWidget->addWidget(tm);
    ui->stackedWidget->setCurrentWidget(tm);
}

void MainWindow::on_actionFlightManager_triggered()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);

        // 【新增】连接信号（管理员模式购票也需要同步订单）
        connect(m_flightManager, &FlightManager::orderCreated,
                m_orderManager, &OrderManager::refreshOrderList);
    }
    m_flightManager->setAdminMode(true); // 管理员模式
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}
void MainWindow::on_btnFlightQuery_clicked()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);

        // 【新增】连接FlightManager的订单创建信号到OrderManager的刷新方法
        connect(m_flightManager, &FlightManager::orderCreated,
                m_orderManager, &OrderManager::refreshOrderList);
    }
    m_flightManager->setAdminMode(false); // 普通查询模式
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}
void MainWindow::on_btnOrderManager_clicked() // 假设主窗口有“订单管理”按钮
{
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
    }
    ui->stackedWidget->setCurrentWidget(m_orderManager);
}
void MainWindow::on_btnOrders_clicked()
{
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
    }
    ui->stackedWidget->setCurrentWidget(m_orderManager);
}
