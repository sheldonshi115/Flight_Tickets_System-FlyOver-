#include "flightmanager.h"
#include "ui_flightmanager.h"
#include "dbmanager.h"
#include "flightdialog.h"
#include "seatdialog.h"
#include <QMessageBox>
#include <QStackedWidget>
#include <ordermanager.h>
#include <QHeaderView>
#include <QDebug>
#include <QRegularExpression>
#include <QUuid>
#include <QTimer> // 新增：延迟刷新

// 构造函数
FlightManager::FlightManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlightManager),
    m_isAdminMode(false),
    m_selectedSeat(""),
    m_currentFlightNo(""),
    m_isManualClick(true) // 初始化手动点击标记
{
    ui->setupUi(this);

    // 1. 初始化和日期设置
    setupTableView();
    onRefreshClicked(); // 初始加载数据

    ui->deDate->setDisplayFormat("yyyy-MM-dd");
    QDate currentDate = QDate::currentDate();
    QDate endDate = currentDate.addMonths(1);

    ui->deDate->setDate(currentDate);
    ui->deDate->setMinimumDate(currentDate);
    ui->deDate->setMaximumDate(endDate);

    // 2. 连接信号槽
    connect(ui->btnAdd, &QPushButton::clicked, this, &FlightManager::onAddFlightClicked);
    connect(ui->btnEdit, &QPushButton::clicked, this, &FlightManager::onEditFlightClicked);
    connect(ui->btnSearch, &QPushButton::clicked, this, &FlightManager::onSearchFlightsClicked);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &FlightManager::onRefreshClicked);

    // 退出按钮逻辑：返回主页
    connect(ui->btnExit, &QPushButton::clicked, this, &FlightManager::on_btnExit_clicked);

    // 购票和查看座位 + 手动点击标记
    connect(ui->btnBook, &QPushButton::clicked, this, [this]() {
        m_isManualClick = true;
        onBookTicketClicked();
    });
    connect(ui->btnSeat, &QPushButton::clicked, this, [this]() {
        m_isManualClick = true;
        onSelectSeatClicked();
    });

    // 连接表格选择变化槽
    connect(ui->twFlightList, &QTableWidget::itemSelectionChanged,
            this, &FlightManager::on_twFlightList_itemSelectionChanged);

    // 接收OrderManager的取消信号，刷新并清空选中（核心修复）
    OrderManager* orderManager = dynamic_cast<OrderManager*>(parent);
    if (orderManager) {
        connect(orderManager, &OrderManager::orderCanceled, this, [this](const QString& flightNum) {
            qDebug() << "收到取消订单信号，刷新航班：" << flightNum;

            // 第一步：强制禁用按钮，阻断校验触发
            ui->btnSeat->setEnabled(false);
            ui->btnBook->setEnabled(false);
            m_isManualClick = false; // 标记为非手动触发

            // 第二步：清空所有状态，彻底取消选中
            m_selectedSeat.clear();
            m_currentFlightNo.clear();
            ui->twFlightList->clearSelection();
            ui->twFlightList->setCurrentItem(nullptr);
            ui->twFlightList->selectRow(-1);
            // 第三步：延迟刷新，避免事件循环拥堵
            QTimer::singleShot(100, this, [this]() {
                onRefreshClicked();
                on_twFlightList_itemSelectionChanged();
            });
        });
    }
}

FlightManager::~FlightManager()
{
    delete ui;
}

void FlightManager::setupTableView()
{
    ui->twFlightList->setColumnCount(9);
    ui->twFlightList->setHorizontalHeaderLabels({
        "ID","航班号", "出发城市", "到达城市",
        "出发时间", "到达时间", "票价",
        "总座位", "可用座位"
    });
    ui->twFlightList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->twFlightList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twFlightList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twFlightList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twFlightList->hideColumn(0); // 隐藏id列
}

void FlightManager::loadFlightsToTable(const QList<Flight>& flights)
{
    // 清空选中项
    ui->twFlightList->clearSelection();
    ui->twFlightList->setCurrentItem(nullptr);

    // 清空原有数据
    ui->twFlightList->setRowCount(0);

    // 填充新数据
    for (const auto& flight : flights) {
        int row = ui->twFlightList->rowCount();
        ui->twFlightList->insertRow(row);

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

    // 同步按钮状态
    on_twFlightList_itemSelectionChanged();
}

Flight FlightManager::getSelectedFlight()
{
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
    Flight defaultFlight;
    defaultFlight.setTotalSeats(180);
    dialog.setFlight(defaultFlight);

    if (dialog.exec() == QDialog::Accepted) {
        Flight newFlight = dialog.getFlight();
        newFlight.setAvailableSeats(newFlight.totalSeats());
        if (DBManager::instance().addFlight(newFlight)) {
            QMessageBox::information(this, "成功", "航班添加成功");
            onRefreshClicked();
        } else {
            QMessageBox::critical(this, "失败", "航班添加失败");
        }
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
    dialog.setFlight(selected);
    if (dialog.exec() == QDialog::Accepted) {
        Flight updatedFlight = dialog.getFlight();
        updatedFlight.setId(selected.id());

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
    QString departure = ui->leDeparture->text().trimmed();
    QString arrival = ui->leArrival->text().trimmed();

    QDate selectedDate = ui->deDate->date();
    QTime zeroTime(0, 0, 0);
    QDateTime date(selectedDate, zeroTime);

    if (selectedDate == ui->deDate->minimumDate()) {
        date = QDateTime();
    }

    QList<Flight> results = DBManager::instance().findFlights(departure, arrival, date);
    loadFlightsToTable(results);
}

void FlightManager::setAdminMode(bool isAdminMode)
{
    m_isAdminMode = isAdminMode;
    ui->btnAdd->setVisible(isAdminMode);
    ui->btnEdit->setVisible(isAdminMode);
    ui->btnDelete->setVisible(isAdminMode);
}

void FlightManager::on_btnDelete_clicked()
{
    QTableWidgetItem* selectedItem = ui->twFlightList->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "提示", "请先选择要删除的航班");
        return;
    }

    int selectedRow = selectedItem->row();
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

void FlightManager::restoreSelectedFlight()
{
    if (m_currentFlightNo.isEmpty()) return;
    for (int i = 0; i < ui->twFlightList->rowCount(); ++i) {
        QString flightNo = ui->twFlightList->item(i, 1)->text();
        if (flightNo == m_currentFlightNo) {
            ui->twFlightList->selectRow(i);
            break;
        }
    }
}

QString FlightManager::generateOrderNumber()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    QString uuid = QUuid::createUuid().toString().mid(1, 8);
    return QString("ORD%1%2").arg(timestamp).arg(uuid);
}

void FlightManager::onBookTicketClicked()
{
    // 非手动点击直接返回（阻断自动触发）
    if (!m_isManualClick) {
        m_isManualClick = true;
        return;
    }

    qDebug() << "===== 进入购票流程 =====";
    Flight flight = getSelectedFlight();
    if (flight.id() == 0 || flight.flightNumber().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择有效航班！");
        return;
    }
    qDebug() << "选中航班：" << flight.flightNumber() << " 可用座位：" << flight.availableSeats();

    if (m_selectedSeat.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择座位！");
        return;
    }
    qDebug() << "已选座位：" << m_selectedSeat;

    if (flight.availableSeats() <= 0) {
        QMessageBox::warning(this, "提示", "该航班已无可用座位！");
        return;
    }

    // 确认购票弹窗
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("确认购票");
    confirmBox.setText(QString("您确定购买 %1 航班的 %2 座位吗？\n票价：¥%3")
                           .arg(flight.flightNumber())
                           .arg(m_selectedSeat)
                           .arg(flight.price(), 0, 'f', 2));
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.button(QMessageBox::Yes)->setText("确认");
    confirmBox.button(QMessageBox::No)->setText("取消");
    confirmBox.setStyleSheet(R"(
        QMessageBox {
            background-color: white;
            color: #222222;
            font-family: "微软雅黑";
            font-size: 14px;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 14px;
        }
        QPushButton#qt_msgbox_no {
            background-color: #666666;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
    )");

    if (confirmBox.exec() != QMessageBox::Yes) return;

    // 更新可用座位
    Flight updatedFlight = flight;
    updatedFlight.setAvailableSeats(flight.availableSeats() - 1);
    if (!DBManager::instance().updateFlight(updatedFlight)) {
        QMessageBox::critical(this, "失败", "座位更新失败，请重试！");
        return;
    }
    qDebug() << "座位更新成功：可用座位变为" << updatedFlight.availableSeats();

    // 生成订单
    Order newOrder;
    newOrder.setOrderNumber(generateOrderNumber());
    newOrder.setFlightNumber(flight.flightNumber());
    newOrder.setDepartureCity(flight.departureCity());
    newOrder.setArrivalCity(flight.arrivalCity());
    newOrder.setDepartTime(flight.departureTime());
    newOrder.setSeatNumber(m_selectedSeat);
    newOrder.setPrice(flight.price());
    newOrder.setStatus("已支付");

    // 写入数据库
    if (!DBManager::instance().addOrder(newOrder)) {
        updatedFlight.setAvailableSeats(flight.availableSeats());
        DBManager::instance().updateFlight(updatedFlight);
        QMessageBox::critical(this, "失败", "订单创建失败，座位已恢复！");
        return;
    }
    qDebug() << "订单创建成功：" << newOrder.orderNumber();

    // 重置状态
    m_selectedSeat.clear();
    m_currentFlightNo.clear();
    ui->btnBook->setEnabled(false);

    // 刷新+发送信号
    onRefreshClicked();
    emit orderCreated();

    // 购票成功弹窗
    QString bookSuccessText = QString("购票成功！\n\n订单号：%1\n航班号：%2\n出发城市：%3\n到达城市：%4\n出发时间：%5\n座位号：%6\n票价：¥%7")
                                  .arg(newOrder.orderNumber())
                                  .arg(flight.flightNumber())
                                  .arg(flight.departureCity())
                                  .arg(flight.arrivalCity())
                                  .arg(flight.departureTime().toString("yyyy-MM-dd HH:mm"))
                                  .arg(newOrder.seatNumber())
                                  .arg(flight.price(), 0, 'f', 2);

    QMessageBox successBox(this);
    successBox.setWindowTitle("购票成功");
    successBox.setText(bookSuccessText);
    successBox.setIcon(QMessageBox::Information);
    successBox.setStandardButtons(QMessageBox::Ok);
    successBox.button(QMessageBox::Ok)->setText("确认");
    successBox.setStyleSheet(R"(
        QMessageBox {
            background-color: white;
            color: #222222;
            font-family: "微软雅黑";
            font-size: 14px;
            padding: 20px;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 20px;
            font-size: 14px;
        }
        QPushButton:hover {
            opacity: 0.9;
        }
    )");
    successBox.exec();
}

void FlightManager::onRefreshClicked()
{
    loadFlightsToTable(DBManager::instance().getAllFlights());
    if (!m_currentFlightNo.isEmpty()) {
        restoreSelectedFlight();
    }
    on_twFlightList_itemSelectionChanged();
}

void FlightManager::onSelectSeatClicked()
{
    // 非手动点击直接返回（阻断自动触发）
    if (!m_isManualClick) {
        m_isManualClick = true;
        return;
    }

    Flight flight = getSelectedFlight();
    if (flight.flightNumber().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择航班");
        return;
    }

    FlightInfo info;
    info.flightNumber = flight.flightNumber();
    info.departureCity = flight.departureCity();
    info.arrivalCity = flight.arrivalCity();
    info.dateTime = flight.departureTime().toString("yyyy-MM-dd HH:mm");

    QVector<SeatData> seats = {
        {"1A", Available, true, false}, {"1B", Sold, false, true},
        {"1C", Available, false, true}, {"1D", Available, true, false},
        {"2A", Available, true, false}, {"2B", Available, false, true},
        {"3A", Sold, true, false},      {"3B", Available, false, false},
        {"4F", Available, true, false}
    };
    info.allSeats = seats;

    SeatDialog dialog(info, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_selectedSeat = dialog.getSelectedSeatId();
        if (!m_selectedSeat.isEmpty()) {
            QString seatSuccessText = QString("选座成功！\n\n航班号：%1\n出发城市：%2\n到达城市：%3\n出发时间：%4\n选中座位：%5")
                                          .arg(flight.flightNumber())
                                          .arg(flight.departureCity())
                                          .arg(flight.arrivalCity())
                                          .arg(flight.departureTime().toString("yyyy-MM-dd HH:mm"))
                                          .arg(m_selectedSeat);

            QMessageBox infoBox(this);
            infoBox.setWindowTitle("选座成功");
            infoBox.setText(seatSuccessText);
            infoBox.setIcon(QMessageBox::Information);
            infoBox.setStandardButtons(QMessageBox::Ok);
            infoBox.button(QMessageBox::Ok)->setText("确认");
            infoBox.setStyleSheet(R"(
                QMessageBox {
                    background-color: white;
                    color: #222222;
                    font-family: "微软雅黑";
                    font-size: 14px;
                    padding: 20px;
                }
                QPushButton {
                    background-color: #4CAF50;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 8px 20px;
                    font-size: 14px;
                }
                QPushButton:hover {
                    opacity: 0.9;
                }
            )");
            infoBox.exec();

            on_twFlightList_itemSelectionChanged();
        }
    }
}

void FlightManager::on_twFlightList_itemSelectionChanged()
{
    bool hasSelection = (ui->twFlightList->currentRow() >= 0);
    qDebug() << "===== 状态检查 =====";
    qDebug() << "是否选中航班：" << hasSelection;
    qDebug() << "已选座位：" << m_selectedSeat;
    qDebug() << "当前航班号：" << m_currentFlightNo;

    if (hasSelection) {
        m_currentFlightNo = ui->twFlightList->item(ui->twFlightList->currentRow(), 1)->text();
    } else {
        m_currentFlightNo.clear();
        m_selectedSeat.clear();
    }

    // 按钮状态控制
    ui->btnSeat->setEnabled(hasSelection);
    bool canBook = hasSelection && !m_selectedSeat.isEmpty();
    ui->btnBook->setEnabled(canBook);
    ui->btnBook->setStyleSheet(canBook ?
                                   "background-color: #4CAF50; color: white; border-radius: 6px;" :
                                   "");

    if (m_isAdminMode) {
        ui->btnEdit->setEnabled(hasSelection);
        ui->btnDelete->setEnabled(hasSelection);
    }
}
