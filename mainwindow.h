#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "flightmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    FlightManager* m_flightManager = nullptr; // 声明航班管理窗口指针
private slots:
    void onManageFlightsClicked(); // 声明槽函数
};

#endif // MAINWINDOW_H
