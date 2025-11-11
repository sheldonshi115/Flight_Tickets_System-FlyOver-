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
    connect(ui->btnEdit, &QPushButton::clicked, this, &FlightManager::onEditFlightClicked);
    connect(ui->btnDelete, &QPushButton::clicked, this, &FlightManager::onDeleteFlightClicked);
    connect(ui->btnSearch, &QPushButton::clicked, this, &FlightManager::onSearchFlightsClicked);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &FlightManager::onRefreshClicked);
}

FlightManager::~FlightManager()
{
    delete ui;
}

void FlightManager::setupTableView()
{
    ui->tableView->setColumnCount(8);
    ui->tableView->setHorizontalHeaderLabels({
        "航班号", "出发城市", "到达城市",
        "出发时间", "到达时间", "票价",
        "总座位", "可用座位"
    });
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void FlightManager::loadFlightsToTable(const QList<Flight>& flights)
{
    ui->tableView->setRowCount(0);
    for (const auto& flight : flights) {
        int row = ui->tableView->rowCount();
        ui->tableView->insertRow(row);

        ui->tableView->setItem(row, 0, new QTableWidgetItem(flight.flightNumber()));
        ui->tableView->setItem(row, 1, new QTableWidgetItem(flight.departureCity()));
        ui->tableView->setItem(row, 2, new QTableWidgetItem(flight.arrivalCity()));
        ui->tableView->setItem(row, 3, new QTableWidgetItem(flight.departureTime().toString("yyyy-MM-dd hh:mm")));
        ui->tableView->setItem(row, 4, new QTableWidgetItem(flight.arrivalTime().toString("yyyy-MM-dd hh:mm")));
        ui->tableView->setItem(row, 5, new QTableWidgetItem(QString::number(flight.price(), 'f', 2)));
        ui->tableView->setItem(row, 6, new QTableWidgetItem(QString::number(flight.totalSeats())));
        ui->tableView->setItem(row, 7, new QTableWidgetItem(QString::number(flight.availableSeats())));
    }
}

Flight FlightManager::getSelectedFlight()
{
    int row = ui->tableView->currentRow();
    if (row < 0) return Flight();

    Flight flight;
    flight.setFlightNumber(ui->tableView->item(row, 0)->text());
    flight.setDepartureCity(ui->tableView->item(row, 1)->text());
    flight.setArrivalCity(ui->tableView->item(row, 2)->text());
    flight.setDepartureTime(QDateTime::fromString(ui->tableView->item(row, 3)->text(), "yyyy-MM-dd hh:mm"));
    flight.setArrivalTime(QDateTime::fromString(ui->tableView->item(row, 4)->text(), "yyyy-MM-dd hh:mm"));
    flight.setPrice(ui->tableView->item(row, 5)->text().toDouble());
    flight.setTotalSeats(ui->tableView->item(row, 6)->text().toInt());
    flight.setAvailableSeats(ui->tableView->item(row, 7)->text().toInt());

    return flight;
}

void FlightManager::onAddFlightClicked()
{
    FlightDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        DbManager::instance().addFlight(dialog.getFlight());
        onRefreshClicked();
    }
}

void FlightManager::onEditFlightClicked()
{
    Flight flight = getSelectedFlight();
    if (flight.flightNumber().isEmpty()) return;

    FlightDialog dialog(this, flight);
    if (dialog.exec() == QDialog::Accepted) {
        DbManager::instance().updateFlight(dialog.getFlight());
        onRefreshClicked();
    }
}

void FlightManager::onDeleteFlightClicked()
{
    Flight flight = getSelectedFlight();
    if (flight.flightNumber().isEmpty()) return;

    if (QMessageBox::question(this, "确认删除",
                              QString("确定要删除航班 %1 吗？").arg(flight.flightNumber())) == QMessageBox::Yes) {
        DbManager::instance().removeFlight(flight.flightNumber());
        onRefreshClicked();
    }
}

void FlightManager::onSearchFlightsClicked()
{
    QString departure = ui->txtDeparture->text();
    QString arrival = ui->txtArrival->text();
    QDateTime date = ui->dateEdit->dateTime();

    QList<Flight> results = DbManager::instance().findFlights(departure, arrival, date);
    loadFlightsToTable(results);
}

void FlightManager::onRefreshClicked()
{
    loadFlightsToTable(DbManager::instance().getAllFlights());
}
