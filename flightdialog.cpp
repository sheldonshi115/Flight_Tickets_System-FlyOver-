#include "flightdialog.h"
#include "ui_flightdialog.h"

FlightDialog::FlightDialog(QWidget *parent, const Flight& flight) :
    QDialog(parent),
    ui(new Ui::FlightDialog)
{
    ui->setupUi(this);

    if (!flight.flightNumber().isEmpty()) {
        setWindowTitle("编辑航班");
        ui->txtFlightNumber->setText(flight.flightNumber());
        ui->txtFlightNumber->setReadOnly(true); // 航班号不可修改
        ui->txtDeparture->setText(flight.departureCity());
        ui->txtArrival->setText(flight.arrivalCity());
        ui->dateTimeEditDeparture->setDateTime(flight.departureTime());
        ui->dateTimeEditArrival->setDateTime(flight.arrivalTime());
        ui->doubleSpinBoxPrice->setValue(flight.price());
        ui->spinBoxTotalSeats->setValue(flight.totalSeats());
        ui->spinBoxAvailableSeats->setValue(flight.availableSeats());
    } else {
        setWindowTitle("新增航班");
    }
}

FlightDialog::~FlightDialog()
{
    delete ui;
}

Flight FlightDialog::getFlight() const
{
    Flight flight;
    flight.setFlightNumber(ui->txtFlightNumber->text());
    flight.setDepartureCity(ui->txtDeparture->text());
    flight.setArrivalCity(ui->txtArrival->text());
    flight.setDepartureTime(ui->dateTimeEditDeparture->dateTime());
    flight.setArrivalTime(ui->dateTimeEditArrival->dateTime());
    flight.setPrice(ui->doubleSpinBoxPrice->value());
    flight.setTotalSeats(ui->spinBoxTotalSeats->value());
    flight.setAvailableSeats(ui->spinBoxAvailableSeats->value());
    return flight;
}
