#ifndef SEATDIALOG_H
#define SEATDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVector>
#include <QString>

namespace Ui {
class SeatDialog;
}

enum SeatState {
    Available, // 可选（绿色）
    Sold,      // 已售（红色）
    Selected   // 已选（黄色）
};

struct SeatData {
    QString seatId;    // 座位号（如"1A"）
    SeatState state;
    bool isWindow;     // 靠窗
    bool isAisle;      // 靠过道
};

struct FlightInfo {
    QString flightNumber;
    QString departureCity;
    QString arrivalCity;
    QString dateTime;
    QVector<SeatData> allSeats; // 所有座位数据
};

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
