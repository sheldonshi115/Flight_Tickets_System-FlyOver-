#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "flightmanager.h"

void MainWindow::onManageFlightsClicked()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(this);
        ui->stackedWidget->addWidget(m_flightManager); // 假设使用stackedWidget切换界面
    }
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);  // 加载UI布局（初始为空）
    this->setWindowTitle("Flight Tickets System - FlyOver");  // 设置窗口标题
}

MainWindow::~MainWindow() {
    delete ui;
}
