#include "mainwindow.h"
#include "ui_mainwindow.h"
#include"dialog_login.h"
#include<QMessageBox>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);  // 加载UI布局（初始为空）
    this->setWindowTitle("Flight Tickets System - FlyOver");  // 设置窗口标题
}
void MainWindow::ShowLogin_Dialog(){
    Dialog_login dialog(this);
    if (dialog.exec()==QDialog::Accepted||dialog.exec()==QDialog::Rejected){
    }
}
void MainWindow::on_btnlogin_clicked(){
    MainWindow::ShowLogin_Dialog();
}
MainWindow::~MainWindow() {
    delete ui;
}
