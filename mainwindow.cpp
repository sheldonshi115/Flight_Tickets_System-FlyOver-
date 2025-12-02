#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "views/travelmoment.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_flightManager(nullptr), // 初始化指针
    m_isAdmin(false) // 默认非管理员，实际应从登录信息获取
{
    ui->setupUi(this);
    connect(ui->travelbutton, &QPushButton::clicked, this, &MainWindow::onTravelButtonClicked);
    connect(ui->btnHome, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageHome); // 主页返回逻辑（保留）
    });
    // 绑定左侧“航班查询”按钮
    connect(ui->btnFlightQuery, &QPushButton::clicked, this, &MainWindow::on_btnFlightQuery_clicked);
    setWindowTitle("航班票务系统 - 主菜单");

    setIsAdmin(true);
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

// 航班管理菜单（管理员模式）
void MainWindow::on_actionFlightManager_triggered()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);
    }
    m_flightManager->setAdminMode(true); // 管理员模式
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}

// 航班查询按钮（普通模式）
void MainWindow::on_btnFlightQuery_clicked()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);
    }
    m_flightManager->setAdminMode(false); // 普通查询模式
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}
