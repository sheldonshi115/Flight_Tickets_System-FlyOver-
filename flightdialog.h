// flightdialog.h
#ifndef FLIGHTDIALOG_H
#define FLIGHTDIALOG_H

#include <QDialog>
#include "flight.h"

namespace Ui {
class FlightDialog;
}

class FlightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FlightDialog(QWidget *parent = nullptr);
    ~FlightDialog();

    // 用于编辑航班时，向对话框传递数据
    void setFlight(const Flight& flight);

    // 获取用户输入或修改后的航班数据
    Flight getFlight() const;

private slots:
    // 覆盖 QDialog::accept()，用于数据验证
    void accept() override;

private:
    Ui::FlightDialog *ui;
};

#endif // FLIGHTDIALOG_H
