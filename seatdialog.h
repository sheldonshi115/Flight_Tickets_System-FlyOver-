#ifndef SEATDIALOG_H
#define SEATDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVector>
#include <QString>
#include<commondefs.h>

namespace Ui {
class SeatDialog;
}


class SeatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SeatDialog(const FlightInfo& flightInfo, QWidget *parent = nullptr);
    ~SeatDialog();
    QString getSelectedSeatId() const;

private slots:
    void handleSeatClicked();
    void on_btnConfirm_clicked();

private:
    Ui::SeatDialog *ui;
    FlightInfo m_flightInfo;
    QPushButton* m_selectedSeat = nullptr;

    void initFlightInfoLabel();
    void createSeatLayout(); // 生成座位布局
    void updateButtonStyle(QPushButton* btn, SeatState state); // 更新座位样式
};

#endif // SEATDIALOG_H
