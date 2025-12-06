#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "views/travelmoment.h"
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QApplication>
#include "ai.h"
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_flightManager(nullptr),
    m_aiquery(nullptr),
    m_ticketBooking(nullptr),
    m_orderManagement(nullptr),
    m_dataAnalytics(nullptr),
    m_isAdmin(false),
    m_isDarkMode(false)
{
    ui->setupUi(this);

    connect(ui->btnHome, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageHome);
    });
    connect(ui->travelbutton, &QPushButton::clicked, this, &MainWindow::onTravelButtonClicked);
    connect(ui->btnFlightQuery, &QPushButton::clicked, this, &MainWindow::on_btnFlightQuery_clicked);
    connect(ui->actionToggleTheme, &QAction::triggered, this, &MainWindow::toggleTheme);
    setWindowTitle("航班票务系统 - 主菜单");

    setIsAdmin(true);
    applyTheme(false); // 默认浅色主题
}

void MainWindow::on_btnTicketBooking_clicked()
{
    if (!m_ticketBooking) {
        m_ticketBooking = new TicketBookingWidget(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_ticketBooking);
    }
    ui->stackedWidget->setCurrentWidget(m_ticketBooking);
}

void MainWindow::on_btnOrderManagement_clicked()
{
    if (!m_orderManagement) {
        m_orderManagement = new OrderManagementWidget(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManagement);
    }
    ui->stackedWidget->setCurrentWidget(m_orderManagement);
}

void MainWindow::on_btnDataAnalytics_clicked()
{
    if (!m_dataAnalytics) {
        m_dataAnalytics = new DataAnalyticsWidget(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_dataAnalytics);
    }
    ui->stackedWidget->setCurrentWidget(m_dataAnalytics);
}

void MainWindow::on_btnAIService_clicked()
{
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

void MainWindow::applyTheme(bool isDarkMode)
{
    m_isDarkMode = isDarkMode;

    if (isDarkMode) {
        // 深色主题
        QString darkStyle = R"(
            QMainWindow { background-color: #1e1e1e; color: #e0e0e0; }
            QWidget { background-color: #1e1e1e; color: #e0e0e0; }
            QPushButton { background-color: #0078d4; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }
            QPushButton:hover { background-color: #106ebe; }
            QLineEdit, QComboBox, QDateEdit { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #444; border-radius: 4px; padding: 6px; }
            QListWidget { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #444; border-radius: 4px; }
            QListWidget::item:selected { background-color: #0078d4; }
            QLabel { color: #e0e0e0; }
            QMenuBar { background-color: #2d2d2d; color: #e0e0e0; border-bottom: 1px solid #444; }
            QMenuBar::item:selected { background-color: #0078d4; }
            QMenu { background-color: #2d2d2d; color: #e0e0e0; border: 1px solid #444; }
            QMenu::item:selected { background-color: #0078d4; }
            QGroupBox { color: #e0e0e0; border: 1px solid #444; border-radius: 4px; }
            QTabWidget::pane { border: 1px solid #444; }
            QTabBar::tab { background-color: #2d2d2d; color: #e0e0e0; padding: 6px 20px; border: 1px solid #444; }
            QTabBar::tab:selected { background-color: #0078d4; color: white; }
        )";
        qApp->setStyleSheet(darkStyle);
        ui->actionToggleTheme->setText("切换为浅色主题 (Ctrl+T)");
    } else {
        // 浅色主题
        QString lightStyle = R"(
            QMainWindow { background-color: #ffffff; color: #333333; }
            QWidget { background-color: #f5f5f5; color: #333333; }
            QPushButton { background-color: #0078d4; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; }
            QPushButton:hover { background-color: #106ebe; }
            QLineEdit, QComboBox, QDateEdit { background-color: #ffffff; color: #333333; border: 1px solid #ddd; border-radius: 4px; padding: 6px; }
            QListWidget { background-color: #ffffff; color: #333333; border: 1px solid #ddd; border-radius: 4px; }
            QListWidget::item:selected { background-color: #0078d4; color: white; }
            QLabel { color: #333333; }
            QMenuBar { background-color: #ffffff; color: #333333; border-bottom: 1px solid #ddd; }
            QMenuBar::item:selected { background-color: #0078d4; color: white; }
            QMenu { background-color: #ffffff; color: #333333; border: 1px solid #ddd; }
            QMenu::item:selected { background-color: #0078d4; color: white; }
            QGroupBox { color: #333333; border: 1px solid #ddd; border-radius: 4px; }
            QTabWidget::pane { border: 1px solid #ddd; }
            QTabBar::tab { background-color: #f0f0f0; color: #333333; padding: 6px 20px; border: 1px solid #ddd; }
            QTabBar::tab:selected { background-color: #0078d4; color: white; }
        )";
        qApp->setStyleSheet(lightStyle);
        ui->actionToggleTheme->setText("切换为深色主题 (Ctrl+T)");
    }
}

void MainWindow::toggleTheme()
{
    applyTheme(!m_isDarkMode);
    // 更新菜单项文本以反映当前主题
    if (m_isDarkMode) {
        ui->actionToggleTheme->setText("切换为浅色主题 (Ctrl+T)");
    } else {
        ui->actionToggleTheme->setText("切换为深色主题 (Ctrl+T)");
    }
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
