#include "flightmanager.h"
#include "ui_flightmanager.h"
#include "dbmanager.h"
#include "flightdialog.h"
#include <QMessageBox>

FlightManager::FlightManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlightManager)
{
    ui->setupUi(this);
    setupTableView();
    onRefreshClicked();

    connect(ui->btnAdd, &QPushButton::clicked, this, &FlightManager::onAddFlightClicked);
    connect(ui->btnSearch, &QPushButton::clicked, this, &FlightManager::onSearchFlightsClicked);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &FlightManager::onRefreshClicked);
    connect(ui->exitButton,&QPushButton::clicked,this,&FlightManager::onExitClicked);
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

void FlightManager::onExitClicked(){
    this->close();
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
