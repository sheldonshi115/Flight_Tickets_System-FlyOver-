// flightdialog.cpp
#include "flightdialog.h"
#include "ui_flightdialog.h"
#include <QMessageBox>

FlightDialog::FlightDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlightDialog)
{
    ui->setupUi(this);

    // 设置数据输入控件的默认/限制值
    ui->dtDepartureTime->setCalendarPopup(true);
    ui->dtDepartureTime->setDisplayFormat("yyyy-MM-dd hh:mm");
    ui->dtDepartureTime->setDateTime(QDateTime::currentDateTime());

    ui->dtArrivalTime->setCalendarPopup(true);
    ui->dtArrivalTime->setDisplayFormat("yyyy-MM-dd hh:mm");

    // 修正：确保这里没有导致问题的 QDateTime::addHours() 调用。
    // 在构造时，到达时间默认设置为当前时间即可，让用户自己调整。
    ui->dtArrivalTime->setDateTime(QDateTime::currentDateTime());

    ui->dsPrice->setRange(1.0, 99999.0); // 票价范围
    ui->sbTotalSeats->setRange(1, 500); // 总座位数
    ui->sbAvailableSeats->setRange(0, 500); // 可用座位数

    // 连接信号：QDialogButtonBox的accepted信号连接到我们覆盖的accept()槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FlightDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

FlightDialog::~FlightDialog()
{
    delete ui;
}

void FlightDialog::setFlight(const Flight& flight)
{
    // 根据 ID 判断是新增还是编辑
    bool isEditing = (flight.id() != 0);

    if (isEditing) {
        // 编辑模式下，限制 TotalSeats 的最小值，防止减小到已售座位以下
        ui->sbTotalSeats->setMinimum(flight.totalSeats() - flight.availableSeats());
        ui->sbAvailableSeats->setEnabled(true);
        ui->sbAvailableSeats->setMaximum(flight.totalSeats() + 100); // 暂设一个较大的最大值
    } else {
        // 新增模式
        ui->sbTotalSeats->setMinimum(1);
        ui->sbAvailableSeats->setEnabled(false); // 新增时，可用座位数由 FlightManager 自动设置为等于总座位数
    }

    ui->leFlightNumber->setText(flight.flightNumber());
    ui->leDepartureCity->setText(flight.departureCity());
    ui->leArrivalCity->setText(flight.arrivalCity());
    ui->dtDepartureTime->setDateTime(flight.departureTime());
    ui->dtArrivalTime->setDateTime(flight.arrivalTime());
    ui->dsPrice->setValue(flight.price());
    ui->sbTotalSeats->setValue(flight.totalSeats() > 0 ? flight.totalSeats() : 180);
    ui->sbAvailableSeats->setValue(flight.availableSeats());
}

Flight FlightDialog::getFlight() const
{
    Flight flight;
    flight.setFlightNumber(ui->leFlightNumber->text().trimmed());
    flight.setDepartureCity(ui->leDepartureCity->text().trimmed());
    flight.setArrivalCity(ui->leArrivalCity->text().trimmed());
    flight.setDepartureTime(ui->dtDepartureTime->dateTime());
    flight.setArrivalTime(ui->dtArrivalTime->dateTime());
    flight.setPrice(ui->dsPrice->value());
    flight.setTotalSeats(ui->sbTotalSeats->value());
    // 只有在启用时才读取 AvailableSeats 的值
    flight.setAvailableSeats(ui->sbAvailableSeats->isEnabled() ? ui->sbAvailableSeats->value() : ui->sbTotalSeats->value());
    return flight;
}

void FlightDialog::accept()
{
    // 1. 必填项检查
    if (ui->leFlightNumber->text().trimmed().isEmpty() ||
        ui->leDepartureCity->text().trimmed().isEmpty() ||
        ui->leArrivalCity->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "警告", "航班号、出发城市和到达城市不能为空。");
        return;
    }

    // 2. 时间逻辑检查
    if (ui->dtDepartureTime->dateTime() >= ui->dtArrivalTime->dateTime()) {
        QMessageBox::warning(this, "警告", "到达时间必须晚于出发时间。");
        return;
    }

    // 3. 座位逻辑检查
    if (ui->sbAvailableSeats->value() > ui->sbTotalSeats->value())
    {
        QMessageBox::warning(this, "警告", "可用座位数不能大于总座位数。");
        return;
    }

    // 验证通过，调用基类的 accept() 关闭对话框
    QDialog::accept();
}
