#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->travelbutton, &QPushButton::clicked, this, &MainWindow::onTravelButtonClicked);
    setWindowTitle("航班票务系统 - 主菜单");
    // 主界面不添加任何航班管理相关控件
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onTravelButtonClicked()
{

}
// 点击“航班管理”菜单后，打开独立的FlightManager窗口
void MainWindow::on_actionFlightManager_triggered()
{
    // 不设置父控件（或传nullptr），使其成为独立窗口
    FlightManager *flightManager = new FlightManager(nullptr);
    flightManager->setWindowTitle("航班管理");
    flightManager->setAttribute(Qt::WA_DeleteOnClose); // 关闭窗口时自动释放内存
    flightManager->show();
}
