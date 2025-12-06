// ticketbooking.h
#ifndef TICKETBOOKING_H
#define TICKETBOOKING_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>

class TicketBookingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TicketBookingWidget(QWidget *parent = nullptr);
    ~TicketBookingWidget();

private slots:
    void onSearchFlights();
    void onFlightListItemClicked();
    void onBookTicket();

private:
    void setupUI();
    QGroupBox* setupSearchPanel();
    QGroupBox* setupFlightResults();
    void loadSampleFlights();
    void loadFlightResults();

    int m_selectedFlightIndex;

    // 搜索面板
    QComboBox *m_fromCity;
    QComboBox *m_toCity;
    QDateEdit *m_departDate;
    QDateEdit *m_returnDate;
    QPushButton *m_searchBtn;
    QListWidget *m_filterList;

    // 结果面板
    QListWidget *m_flightList;
    QPushButton *m_bookBtn;
    QLabel *m_selectedFlightInfo;

    // 数据存储
    struct FlightInfo {
        QString flightNumber;
        QString airline;
        QString departure;
        QString arrival;
        QString departTime;
        QString arriveTime;
        double price;
        int passengers;
        QString type;
    };

    QList<FlightInfo> m_searchResults;
};

#endif // TICKETBOOKING_H
