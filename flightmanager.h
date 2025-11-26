#ifndef FLIGHTMANAGER_H
#define FLIGHTMANAGER_H

#include <QWidget>
#include "flight.h"

namespace Ui {
class FlightManager;
}

class FlightManager : public QWidget
{
    Q_OBJECT

public:
    explicit FlightManager(QWidget *parent = nullptr);
    ~FlightManager();

private slots:
    void onAddFlightClicked();
    void onSearchFlightsClicked();
    void onRefreshClicked();
    void onExitClicked();

private:
    Ui::FlightManager *ui;
    void setupTableView();
    void loadFlightsToTable(const QList<Flight>& flights);
    Flight getSelectedFlight();
};

#endif // FLIGHTMANAGER_H
