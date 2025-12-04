// flightmanager.cpp
#include "flightmanager.h"
#include "ui_flightmanager.h"
#include "dbmanager.h"
#include "flightdialog.h"
#include "seatdialog.h"
#include <QMessageBox>
#include <QStackedWidget>
#include <QHeaderView> // 用于 QHeaderView::Stretch
#include <QDebug>

// 构造函数
FlightManager::FlightManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlightManager)
{
    ui->setupUi(this);

    // 1. 初始化和日期设置
    // 注意：已将 'dateEdit' 统一为 'deDate' (与我提供的UI代码一致)
    setupTableView();
    onRefreshClicked(); // 初始加载数据

    ui->deDate->setDisplayFormat("yyyy-MM-dd");
    QDate currentDate = QDate::currentDate();
    QDate endDate = currentDate.addMonths(1);

    ui->deDate->setDate(currentDate);
    ui->deDate->setMinimumDate(currentDate);
    ui->deDate->setMaximumDate(endDate);

    // 2. 连接信号槽
    // 注意：已将 UI 控件名统一为 UI 代码中的名称 (btnAdd, btnEdit, btnSearch, btnRefresh, btnDelete)
    connect(ui->btnAdd, &QPushButton::clicked, this, &FlightManager::onAddFlightClicked);
    connect(ui->btnEdit, &QPushButton::clicked, this, &FlightManager::onEditFlightClicked);
    connect(ui->btnSearch, &QPushButton::clicked, this, &FlightManager::onSearchFlightsClicked);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &FlightManager::onRefreshClicked);
    connect(ui->btnDelete, &QPushButton::clicked, this, &FlightManager::on_btnDelete_clicked);

    // 退出按钮逻辑：返回主页
    connect(ui->btnExit, &QPushButton::clicked, this, &FlightManager::on_btnExit_clicked);

    //购票和查看座位
    connect(ui->btnBook, &QPushButton::clicked, this, &FlightManager::onBookTicketClicked);
    connect(ui->btnSeat, &QPushButton::clicked, this, &FlightManager::onSelectSeatClicked);

    // 连接表格选择变化槽
    connect(ui->twFlightList, &QTableWidget::itemSelectionChanged,
            this, &FlightManager::on_twFlightList_itemSelectionChanged);
}

FlightManager::~FlightManager()
{
    delete ui;
}

void FlightManager::setupTableView()
{
    // 注意：已将 'tableView' 统一为 'twFlightList' (与我提供的UI代码一致)
    ui->twFlightList->setColumnCount(9);
    ui->twFlightList->setHorizontalHeaderLabels({
        "ID","航班号", "出发城市", "到达城市",
        "出发时间", "到达时间", "票价",
        "总座位", "可用座位"
    });
    // 假设您希望表格自动伸展以填充可用空间，但 QTableWidget 需要一个 QHeaderView
    ui->twFlightList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twFlightList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twFlightList->hideColumn(0); // 隐藏id列
}

void FlightManager::loadFlightsToTable(const QList<Flight>& flights)
{
    // 注意：已将 'tableView' 统一为 'twFlightList'
    ui->twFlightList->setRowCount(0);
    for (const auto& flight : flights) {
        int row = ui->twFlightList->rowCount();
        ui->twFlightList->insertRow(row);

        // 使用 QTableWidgetItem 填充数据
        ui->twFlightList->setItem(row, 0, new QTableWidgetItem(QString::number(flight.id())));
        ui->twFlightList->setItem(row, 1, new QTableWidgetItem(flight.flightNumber()));
        ui->twFlightList->setItem(row, 2, new QTableWidgetItem(flight.departureCity()));
        ui->twFlightList->setItem(row, 3, new QTableWidgetItem(flight.arrivalCity()));
        ui->twFlightList->setItem(row, 4, new QTableWidgetItem(flight.departureTime().toString("yyyy-MM-dd hh:mm")));
        ui->twFlightList->setItem(row, 5, new QTableWidgetItem(flight.arrivalTime().toString("yyyy-MM-dd hh:mm")));
        ui->twFlightList->setItem(row, 6, new QTableWidgetItem(QString::number(flight.price(), 'f', 2)));
        ui->twFlightList->setItem(row, 7, new QTableWidgetItem(QString::number(flight.totalSeats())));
        ui->twFlightList->setItem(row, 8, new QTableWidgetItem(QString::number(flight.availableSeats())));
    }
}

Flight FlightManager::getSelectedFlight()
{
    // 注意：已将 'tableView' 统一为 'twFlightList'
    int row = ui->twFlightList->currentRow();
    if (row < 0) return Flight();

    Flight flight;
    flight.setId(ui->twFlightList->item(row, 0)->text().toInt());
    flight.setFlightNumber(ui->twFlightList->item(row, 1)->text());
    flight.setDepartureCity(ui->twFlightList->item(row, 2)->text());
    flight.setArrivalCity(ui->twFlightList->item(row, 3)->text());
    flight.setDepartureTime(QDateTime::fromString(ui->twFlightList->item(row, 4)->text(), "yyyy-MM-dd hh:mm"));
    flight.setArrivalTime(QDateTime::fromString(ui->twFlightList->item(row, 5)->text(), "yyyy-MM-dd hh:mm"));
    flight.setPrice(ui->twFlightList->item(row, 6)->text().toDouble());
    flight.setTotalSeats(ui->twFlightList->item(row, 7)->text().toInt());
    flight.setAvailableSeats(ui->twFlightList->item(row, 8)->text().toInt());

    return flight;
}

void FlightManager::onAddFlightClicked()
{
    FlightDialog dialog(this);
    dialog.setWindowTitle("新增航班");
    // 设置默认值，让对话框知道是新增模式
    Flight defaultFlight;
    defaultFlight.setTotalSeats(180);
    dialog.setFlight(defaultFlight);

    if (dialog.exec() == QDialog::Accepted) {
        Flight newFlight = dialog.getFlight();
        newFlight.setAvailableSeats(newFlight.totalSeats()); // 可用座位默认等于总座位
        if (DBManager::instance().addFlight(newFlight)) {
            QMessageBox::information(this, "成功", "航班添加成功");
            onRefreshClicked();
        } else {
            QMessageBox::critical(this, "失败", "航班添加失败");
        }
    }
}

void FlightManager::onBookTicketClicked()
{
    Flight flight = getSelectedFlight();
    if (!flight.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择要购买的航班");
        return;
    }

    // 检查可用座位
    if (flight.availableSeats() <= 0) {
        QMessageBox::warning(this, "无票", "该航班已无可用座位");
        return;
    }

    // 弹出购票确认对话框
    if (QMessageBox::question(this, "确认购票",
                              QString("航班信息：\n%1 %2 -> %3\n出发时间：%4\n票价：¥%5\n\n确定购买吗？")
                                  .arg(flight.flightNumber())
                                  .arg(flight.departureCity())
                                  .arg(flight.arrivalCity())
                                  .arg(flight.departureTime().toString("yyyy-MM-dd HH:mm"))
                                  .arg(flight.price()),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return; // 用户取消购票
    }

    // 核心逻辑：更新可用座位（减1）
    Flight updatedFlight = flight; // 复制当前航班信息
    updatedFlight.setAvailableSeats(flight.availableSeats() - 1); // 可用座位-1

    // 调用数据库更新
    if (DBManager::instance().updateFlight(updatedFlight)) {
        QMessageBox::information(this, "购票成功",
                                 QString("已成功购买 %1 航班的机票！\n剩余可用座位：%2")
                                     .arg(flight.flightNumber())
                                     .arg(updatedFlight.availableSeats()));
        onRefreshClicked(); // 刷新表格，显示最新可用座位
    } else {
        QMessageBox::critical(this, "购票失败", "数据库更新失败，请重试");
    }
}

void FlightManager::onEditFlightClicked()
{
    Flight selected = getSelectedFlight();
    if (selected.id() == 0) {
        QMessageBox::warning(this, "提示", "请先选择要编辑的航班");
        return;
    }

    FlightDialog dialog(this);
    dialog.setWindowTitle("编辑航班");
    dialog.setFlight(selected); // 初始化对话框数据
    if (dialog.exec() == QDialog::Accepted) {
        Flight updatedFlight = dialog.getFlight();
        updatedFlight.setId(selected.id()); // 保持ID不变

        // 确保可用座位不超过总座位
        if (updatedFlight.availableSeats() > updatedFlight.totalSeats()) {
            updatedFlight.setAvailableSeats(updatedFlight.totalSeats());
        }

        if (DBManager::instance().updateFlight(updatedFlight)) {
            QMessageBox::information(this, "成功", "航班更新成功");
            onRefreshClicked();
        } else {
            QMessageBox::critical(this, "失败", "航班更新失败");
        }
    }
}

void FlightManager::onSearchFlightsClicked()
{
    // 注意：已将 'txtDeparture'/'txtArrival' 统一为 'leDeparture'/'leArrival'
    QString departure = ui->leDeparture->text().trimmed();
    QString arrival = ui->leArrival->text().trimmed();

    // 注意：已将 'dateEdit' 统一为 'deDate'
    QDate selectedDate = ui->deDate->date();
    QTime zeroTime(0, 0, 0);
    QDateTime date(selectedDate, zeroTime);

    // 检查是否为最小日期（即当天），如果是，则不限制日期查询
    if (selectedDate == ui->deDate->minimumDate()) {
        date = QDateTime();
    }

    QList<Flight> results = DBManager::instance().findFlights(departure, arrival, date);
    loadFlightsToTable(results);
}

void FlightManager::onSelectSeatClicked()
{
    Flight flight = getSelectedFlight();

    // 1. 检查航班有效性
    if (flight.flightNumber().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择航班");
        return;
    }

    // 2. 构造 FlightInfo 结构体
    FlightInfo info;
    info.flightNumber = flight.flightNumber();
    info.departureCity = flight.departureCity();
    info.arrivalCity = flight.arrivalCity();
    info.dateTime = flight.departureTime().toString("yyyy-MM-dd HH:mm");

    // 3. 【关键步骤：获取座位数据】
    //    由于我们没有您的数据库管理器实现，这里使用模拟数据。
    //    在您的实际项目中，您需要调用 DBManager 来获取该航班的实际座位列表。

    // --- 模拟座位数据 (请替换为实际的数据库调用) ---
    QVector<SeatData> seats = {
        {"1A", Available, true, false}, {"1B", Sold, false, true},
        {"1C", Available, false, true}, {"1D", Available, true, false},
        {"2A", Available, true, false}, {"2B", Available, false, true},
        {"3A", Sold, true, false},      {"3B", Available, false, false},
        {"4F", Available, true, false}
    };
    info.allSeats = seats;
    // --- 模拟数据结束 ---

    // 4. 实例化 SeatDialog，使用构造好的 FlightInfo 结构体
    SeatDialog dialog(info, this); // 现在参数类型匹配

    // 5. 显示对话框并处理结果
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedSeat = dialog.getSelectedSeatId();
        if (!selectedSeat.isEmpty()) {
            QMessageBox::information(this, "选座成功", "您选择了座位：" + selectedSeat);

            // TODO: 在这里调用 DBManager 更新数据库中的座位状态和购票记录
            // 例如：DBManager::instance().updateSeatStatus(flight.id(), selectedSeat, SeatState::Sold);
            onRefreshClicked(); // 刷新航班列表 (如果座位数量变化)
        }
    }
}

void FlightManager::onRefreshClicked()
{
    loadFlightsToTable(DBManager::instance().getAllFlights());
}

void FlightManager::on_twFlightList_itemSelectionChanged()
{
    // 当表格选择变化时，可以更新底部的详情面板或按钮状态（如禁用/启用 '购票'/'编辑'）
    bool hasSelection = (ui->twFlightList->currentRow() >= 0);

    // 购票和选座的按钮启用
    ui->btnBook->setEnabled(hasSelection);
    ui->btnSeat->setEnabled(hasSelection);

    // 对于管理员按钮，只需要检查是否选中
    if (m_isAdminMode) {
        ui->btnEdit->setEnabled(hasSelection);
        ui->btnDelete->setEnabled(hasSelection);
    }
}

void FlightManager::setAdminMode(bool isAdminMode)
{
    m_isAdminMode = isAdminMode;
    // 注意：已将控件名统一
    ui->btnAdd->setVisible(isAdminMode);
    ui->btnEdit->setVisible(isAdminMode);
    ui->btnDelete->setVisible(isAdminMode);
}

void FlightManager::on_btnDelete_clicked()
{
    // 注意：已将 'tableView' 统一为 'twFlightList'
    QTableWidgetItem* selectedItem = ui->twFlightList->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "提示", "请先选择要删除的航班");
        return;
    }

    int selectedRow = selectedItem->row();
    // ID在第0列
    int flightId = ui->twFlightList->item(selectedRow, 0)->text().toInt();

    if (QMessageBox::question(this, "确认删除",
                              "确定要删除选中的航班吗？",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    if (DBManager::instance().removeFlight(flightId)) {
        QMessageBox::information(this, "成功", "航班删除成功");
        onRefreshClicked();
    } else {
        QMessageBox::critical(this, "失败", "航班删除失败");
    }
}

// 原代码：on_exitButton_clicked（假设按钮objectName是btnExit）
void FlightManager::on_btnExit_clicked()
{
    QWidget *mainWindow = this->topLevelWidget();
    QStackedWidget *stackedWidget = mainWindow->findChild<QStackedWidget*>("stackedWidget");
    QWidget *pageHome = stackedWidget->findChild<QWidget*>("pageHome");
    if (stackedWidget && pageHome) {
        stackedWidget->setCurrentWidget(pageHome);
    } else {
        QMessageBox::warning(this, "错误", "无法找到主页页面");
    }
}
