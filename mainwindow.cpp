#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("航班票务系统 - 主菜单"); // 主窗口标题
}

MainWindow::~MainWindow()
{
    delete ui;
}
