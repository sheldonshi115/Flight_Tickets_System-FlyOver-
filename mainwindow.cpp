#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "login.h"
#include "profiledialog.h"

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

// 退出登录
void MainWindow::on_actionLogout_triggered()
{
    // 关闭当前主界面
    this->close();

    // 打开登录窗口
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        // 登录成功后重新打开主界面（可选，根据业务逻辑调整）
        MainWindow *newMainWindow = new MainWindow;
        newMainWindow->show();
    }
}

// 个人资料菜单点击事件
void MainWindow::on_actionProfile_triggered()
{
    // 注意：currentAccount 需要替换为实际登录的账号（从LoginDialog传递过来）
    QString currentAccount = "当前登录账号";
    ProfileDialog *profileDialog = new ProfileDialog(currentAccount, this);
    profileDialog->exec(); // 模态显示个人资料窗口
}
