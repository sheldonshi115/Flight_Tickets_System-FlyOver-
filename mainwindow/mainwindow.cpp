#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);  // 加载UI布局（初始为空）
    this->setWindowTitle("Flight Tickets System - FlyOver");  // 设置窗口标题
}

MainWindow::~MainWindow() {
    delete ui;
}
