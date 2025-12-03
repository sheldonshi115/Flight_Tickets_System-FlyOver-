#include "flightmanager.h"
#include "ui_flightmanager.h"
#include "dbmanager.h"
#include "flightdialog.h"
#include <QMessageBox>
#include <QStackedWidget>

FlightManager::FlightManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlightManager)
{
    ui->setupUi(this);
    setupTableView();
    onRefreshClicked();
    ui->dateEdit->setDisplayFormat("yyyy-MM-dd");
    QDate currentDate = QDate::currentDate();
    QDate endDate = currentDate.addMonths(1);
    ui->dateEdit->setDate(currentDate);
    ui->dateEdit->setMinimumDate(currentDate);
    ui->dateEdit->setMaximumDate(endDate);
    //connect(ui->btnAdd, &QPushButton::clicked, this, &FlightManager::onAddFlightClicked);
    connect(ui->btnSearch, &QPushButton::clicked, this, &FlightManager::onSearchFlightsClicked);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &FlightManager::onRefreshClicked);
    // 原退出按钮逻辑修改：不再关闭窗口，而是通知主窗口返回主页
    connect(ui->exitButton, &QPushButton::clicked, this, [this]() {
        QWidget *mainWindow = this->topLevelWidget(); // 获取主窗口
        QStackedWidget *stackedWidget = mainWindow->findChild<QStackedWidget*>("stackedWidget");
        QWidget *pageHome = stackedWidget->findChild<QWidget*>("pageHome");
        if (stackedWidget && pageHome) {
            stackedWidget->setCurrentWidget(pageHome);
        }
    });
}

FlightManager::~FlightManager()
{
    delete ui;
}

void FlightManager::setupTableView()
{
    ui->tableView->setColumnCount(9);
    ui->tableView->setHorizontalHeaderLabels({
        "ID","航班号", "出发城市", "到达城市",
        "出发时间", "到达时间", "票价",
        "总座位", "可用座位"
    });
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->hideColumn(0); // 隐藏id列
}

void FlightManager::loadFlightsToTable(const QList<Flight>& flights)
{
    ui->tableView->setRowCount(0);
    for (const auto& flight : flights) {
        int row = ui->tableView->rowCount();
        ui->tableView->insertRow(row);

        ui->tableView->setItem(row, 0, new QTableWidgetItem(QString::number(flight.id())));
        ui->tableView->setItem(row, 1, new QTableWidgetItem(flight.flightNumber()));
        ui->tableView->setItem(row, 2, new QTableWidgetItem(flight.departureCity()));
        ui->tableView->setItem(row, 3, new QTableWidgetItem(flight.arrivalCity()));
        ui->tableView->setItem(row, 4, new QTableWidgetItem(flight.departureTime().toString("yyyy-MM-dd hh:mm")));
        ui->tableView->setItem(row, 5, new QTableWidgetItem(flight.arrivalTime().toString("yyyy-MM-dd hh:mm")));
        ui->tableView->setItem(row, 6, new QTableWidgetItem(QString::number(flight.price(), 'f', 2)));
        ui->tableView->setItem(row, 7, new QTableWidgetItem(QString::number(flight.totalSeats())));
        ui->tableView->setItem(row, 8, new QTableWidgetItem(QString::number(flight.availableSeats())));
    }
}

Flight FlightManager::getSelectedFlight()
{
    int row = ui->tableView->currentRow();
    if (row < 0) return Flight();

    Flight flight;
    flight.setId(ui->tableView->item(row, 0)->text().toInt());
    flight.setFlightNumber(ui->tableView->item(row, 1)->text());
    flight.setDepartureCity(ui->tableView->item(row, 2)->text());
    flight.setArrivalCity(ui->tableView->item(row, 3)->text());
    flight.setDepartureTime(QDateTime::fromString(ui->tableView->item(row, 4)->text(), "yyyy-MM-dd hh:mm"));
    flight.setArrivalTime(QDateTime::fromString(ui->tableView->item(row, 5)->text(), "yyyy-MM-dd hh:mm"));
    flight.setPrice(ui->tableView->item(row, 6)->text().toDouble());
    flight.setTotalSeats(ui->tableView->item(row, 7)->text().toInt());
    flight.setAvailableSeats(ui->tableView->item(row, 8)->text().toInt());

    return flight;
}

void FlightManager::onAddFlightClicked()
{
    FlightDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        DBManager::instance().addFlight(dialog.getFlight());
        onRefreshClicked();
    }
}

void FlightManager::onSearchFlightsClicked()
{
    QString departure = ui->txtDeparture->text().trimmed(); // 去除首尾空格，避免空字符干扰
    QString arrival = ui->txtArrival->text().trimmed();

    // 关键修正：显式传入时间（00:00:00），兼容所有Qt版本
    QDate selectedDate = ui->dateEdit->date(); // 获取选择的日期
    QTime zeroTime(0, 0, 0); // 构造“00:00:00”的时间对象
    QDateTime date(selectedDate, zeroTime); // 同时传QDate+QTime，兼容所有Qt版本

    // 优化：若用户未修改dateEdit（保持默认最小日期），则不限制日期
    if (selectedDate == ui->dateEdit->minimumDate()) {
        date = QDateTime(); // 设为无效QDateTime，跳过日期过滤
    }
    QList<Flight> results = DBManager::instance().findFlights(departure, arrival, date);
    loadFlightsToTable(results);
}

void FlightManager::onRefreshClicked()
{
    loadFlightsToTable(DBManager::instance().getAllFlights());
}

void FlightManager::setAdminMode(bool isAdminMode)
{
    m_isAdminMode = isAdminMode;
    // 管理员模式显示增删按钮，普通模式隐藏
    //ui->btnAdd->setVisible(isAdminMode);
    //ui->btnDelete->setVisible(isAdminMode);
    // 普通模式下，“新增”按钮显示为“选择航班”
    //ui->btnAdd->setText(isAdminMode ? "新增航班" : "选择航班");
}

void FlightManager::on_btnDelete_clicked()
{
    QTableWidgetItem* selectedItem = ui->tableView->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "提示", "请先选择要删除的航班");
        return;
    }
    int selectedRow = selectedItem->row();
    // 假设表格第0列隐藏了航班ID（若实际列不同需调整）
    int flightId = ui->tableView->item(selectedRow, 0)->text().toInt();

    if (QMessageBox::question(this, "确认删除",
                              QString("确定要删除选中的航班吗？"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    bool deleteSuccess = DBManager::instance().removeFlight(flightId);
    if (deleteSuccess) {
        QMessageBox::information(this, "成功", "航班删除成功");
        onRefreshClicked();
    } else {
        QMessageBox::critical(this, "失败", "航班删除失败");
    }
}

void FlightManager::on_exitButton_clicked()
{
    // 找到主窗口的stackedWidget，切换回主页
    QWidget* mainWindow = this->topLevelWidget();
    QStackedWidget* stackedWidget = mainWindow->findChild<QStackedWidget*>("stackedWidget");
    QWidget* pageHome = stackedWidget->findChild<QWidget*>("pageHome");

    if (stackedWidget && pageHome) {
        stackedWidget->setCurrentWidget(pageHome);
    } else {
        QMessageBox::warning(this, "错误", "无法找到主页页面");
    }
}
