#ifndef FLIGHTDIALOG_H
#define FLIGHTDIALOG_H

#include <QDialog>
#include "models/flight.h"
#include "ui_flightdialog.h"

namespace Ui {
class FlightDialog;
}

class FlightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlightDialog(QWidget *parent = nullptr, const Flight& flight = Flight());
    ~FlightDialog();
    Flight getFlight() const;

private:
    Ui::FlightDialog *ui;
};

#endif // FLIGHTDIALOG_H
