#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "flightmanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    FlightManager *m_flightManager; // 成员变量移至private（封装性更好）

private slots:
    void on_actionFlightManager_triggered(); // 新增：菜单点击的槽函数
};

#endif // MAINWINDOW_H
