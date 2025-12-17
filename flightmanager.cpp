#include "flightmanager.h"
#include "ui_flightmanager.h"
#include "dbmanager.h"
#include "flightdialog.h"
#include "seatdialog.h"
#include "membersystem.h"
#include "emailreminder.h"
#include "voucherdialog.h"
#include "orderconfirmdialog.h"
#include <QMessageBox>
#include <QStackedWidget>
#include <ordermanager.h>
#include <QHeaderView>
#include <QDebug>
#include <QRegularExpression>
#include <QUuid>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSet>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>

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

    // 应用现代化样式
    applyModernStyle();

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

    // 连接表格选择变化槽
    connect(ui->twFlightList, &QTableWidget::itemSelectionChanged,
            this, &FlightManager::on_twFlightList_itemSelectionChanged);

    // 接收OrderManager的取消信号，刷新并清空选中（核心修复）
    OrderManager* orderManager = dynamic_cast<OrderManager*>(parent);
    if (orderManager) {
        connect(orderManager, &OrderManager::orderCanceled, this, [this](const QString& flightNum) {
            qDebug() << "收到取消订单信号，刷新航班：" << flightNum;

            // 标记为非手动触发
            m_isManualClick = false;

            // 清空所有状态，彻底取消选中
            m_selectedSeat.clear();
            m_currentFlightNo.clear();
            ui->twFlightList->clearSelection();
            ui->twFlightList->setCurrentItem(nullptr);
            ui->twFlightList->selectRow(-1);
            // 延迟刷新，避免事件循环拥堵
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

void FlightManager::setCurrentUser(const QString& account)
{
    m_currentUserAccount = account;
    qDebug() << "[FlightManager] 当前用户设置为：" << account;
}

void FlightManager::setupTableView()
{
    ui->twFlightList->setColumnCount(9);
    ui->twFlightList->setHorizontalHeaderLabels({
        "ID","航班号", "出发城市", "到达城市",
        "出发时间", "到达时间", "票价",
        "总座位", "可用座位"
    });
    
    // 设置列宽 - 确保时间列能完整显示
    ui->twFlightList->setColumnWidth(1, 100);  // 航班号
    ui->twFlightList->setColumnWidth(2, 100);  // 出发城市
    ui->twFlightList->setColumnWidth(3, 100);  // 到达城市
    ui->twFlightList->setColumnWidth(4, 150);  // 出发时间 - 增宽以显示完整日期时间
    ui->twFlightList->setColumnWidth(5, 150);  // 到达时间 - 增宽以显示完整日期时间
    ui->twFlightList->setColumnWidth(6, 100);  // 票价
    ui->twFlightList->setColumnWidth(7, 80);   // 总座位
    ui->twFlightList->setColumnWidth(8, 80);   // 可用座位
    
    // 其余列自适应
    ui->twFlightList->horizontalHeader()->setStretchLastSection(true);
    
    ui->twFlightList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->twFlightList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->twFlightList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->twFlightList->hideColumn(0); // 隐藏id列
    
    // 现代化表格样式 - 优化版
    ui->twFlightList->setStyleSheet(R"(
        QTableWidget {
            background-color: #FFFFFF;
            border: 2px solid #E2E8F0;
            border-radius: 12px;
            gridline-color: transparent;
            font-size: 13px;
            font-family: 'Microsoft YaHei UI', 'SimHei';
        }
        QTableWidget::item {
            padding: 14px 10px;
            border-bottom: 1px solid #F1F5F9;
            color: #475569;
        }
        QTableWidget::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #DBEAFE, stop:1 #BFDBFE);
            color: #1E40AF;
            font-weight: 600;
        }
        QTableWidget::item:hover {
            background-color: #F8FAFC;
        }
        QHeaderView::section {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #60A5FA, stop:1 #3B82F6);
            color: #FFFFFF;
            font-weight: 700;
            font-size: 13px;
            padding: 14px 10px;
            border: none;
            border-right: 1px solid rgba(255, 255, 255, 0.1);
        }
        QHeaderView::section:first {
            border-top-left-radius: 10px;
        }
        QHeaderView::section:last {
            border-top-right-radius: 10px;
        }
    )");
    
    ui->twFlightList->verticalHeader()->setVisible(false);
    ui->twFlightList->setShowGrid(false);
    ui->twFlightList->setAlternatingRowColors(true);
    ui->twFlightList->setAlternatingRowColors(false); // 禁用默认交替颜色，使用CSS控制
}

void FlightManager::loadFlightsToTable(const QList<Flight>& flights)
{
    // 保存航班列表供卡片视图使用
    m_currentFlights = flights;
    
    // 清空选中项
    ui->twFlightList->clearSelection();
    ui->twFlightList->setCurrentItem(nullptr);

    // 批量更新：禁用绘制与排序以减少重绘和排序开销
    ui->twFlightList->setSortingEnabled(false);
    ui->twFlightList->setUpdatesEnabled(false);

    // 预分配行，避免频繁插入导致的重分配
    int totalRows = flights.size();
    ui->twFlightList->setRowCount(totalRows);

    for (int i = 0; i < totalRows; ++i) {
        const Flight &flight = flights.at(i);
        int row = i;

        // ID列（隐藏）
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(flight.id()));
        ui->twFlightList->setItem(row, 0, idItem);

        // 航班号 - 居中对齐
        QTableWidgetItem* flightNoItem = new QTableWidgetItem(flight.flightNumber());
        flightNoItem->setTextAlignment(Qt::AlignCenter);
        ui->twFlightList->setItem(row, 1, flightNoItem);

        // 出发城市 - 居中对齐
        QTableWidgetItem* depCityItem = new QTableWidgetItem(flight.departureCity());
        depCityItem->setTextAlignment(Qt::AlignCenter);
        ui->twFlightList->setItem(row, 2, depCityItem);

        // 到达城市 - 居中对齐
        QTableWidgetItem* arrCityItem = new QTableWidgetItem(flight.arrivalCity());
        arrCityItem->setTextAlignment(Qt::AlignCenter);
        ui->twFlightList->setItem(row, 3, arrCityItem);

        // 出发时间 - 居中对齐，完整格式
        QTableWidgetItem* depTimeItem = new QTableWidgetItem(
            flight.departureTime().toString("yyyy-MM-dd hh:mm")
        );
        depTimeItem->setTextAlignment(Qt::AlignCenter);
        ui->twFlightList->setItem(row, 4, depTimeItem);

        // 到达时间 - 居中对齐，完整格式
        QTableWidgetItem* arrTimeItem = new QTableWidgetItem(
            flight.arrivalTime().toString("yyyy-MM-dd hh:mm")
        );
        arrTimeItem->setTextAlignment(Qt::AlignCenter);
        ui->twFlightList->setItem(row, 5, arrTimeItem);

        // 票价 - 右对齐，保留2位小数
        QTableWidgetItem* priceItem = new QTableWidgetItem(
            QString::fromUtf8("￥") + QString::number(flight.price(), 'f', 2)
        );
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->twFlightList->setItem(row, 6, priceItem);

        // 总座位 - 居中对齐
        QTableWidgetItem* totalSeatsItem = new QTableWidgetItem(QString::number(flight.totalSeats()));
        totalSeatsItem->setTextAlignment(Qt::AlignCenter);
        ui->twFlightList->setItem(row, 7, totalSeatsItem);

        // 可用座位 - 居中对齐，根据座位情况设置颜色
        QTableWidgetItem* availSeatsItem = new QTableWidgetItem(QString::number(flight.availableSeats()));
        availSeatsItem->setTextAlignment(Qt::AlignCenter);

        if (flight.availableSeats() < 10) {
            availSeatsItem->setForeground(QBrush(QColor("#DC2626")));
            availSeatsItem->setFont(QFont("Microsoft YaHei UI", -1, QFont::Bold));
        } else if (flight.availableSeats() < 50) {
            availSeatsItem->setForeground(QBrush(QColor("#F59E0B")));
        } else {
            availSeatsItem->setForeground(QBrush(QColor("#10B981")));
        }

        ui->twFlightList->setItem(row, 8, availSeatsItem);
    }

    // 恢复绘制与排序
    ui->twFlightList->setUpdatesEnabled(true);
    ui->twFlightList->setSortingEnabled(true);

    // 同步按钮状态
    on_twFlightList_itemSelectionChanged();
    
    // 如果处于卡片视图模式，同时更新卡片
    if (m_isCardViewMode) {
        loadFlightsToCards(flights);
    }
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
    
    // 修复：移除价格字符串中的货币符号
    QString priceStr = ui->twFlightList->item(row, 6)->text();
    priceStr.remove(QRegularExpression("[^0-9.]")); // 移除所有非数字和小数点的字符
    flight.setPrice(priceStr.toDouble());
    
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
    
    // 普通用户模式切换到卡片视图
    setCardViewMode(!isAdminMode);
}

// 设置卡片视图模式
void FlightManager::setCardViewMode(bool cardMode)
{
    m_isCardViewMode = cardMode;
    
    if (cardMode) {
        // 隐藏表格，显示卡片视图
        ui->twFlightList->setVisible(false);
        
        // 创建卡片滚动区域（如果尚未创建）
        if (!m_cardScrollArea) {
            m_cardScrollArea = new QScrollArea(this);
            m_cardScrollArea->setWidgetResizable(true);
            m_cardScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            m_cardScrollArea->setStyleSheet(R"(
                QScrollArea {
                    border: none;
                    background-color: transparent;
                }
                QScrollBar:vertical {
                    background-color: transparent;
                    width: 8px;
                    border-radius: 4px;
                }
                QScrollBar::handle:vertical {
                    background-color: #BDBDBD;
                    border-radius: 4px;
                    min-height: 30px;
                }
                QScrollBar::handle:vertical:hover {
                    background-color: #9E9E9E;
                }
            )");
            
            m_cardContainer = new QWidget();
            m_cardContainer->setStyleSheet("background-color: transparent;");
            m_cardScrollArea->setWidget(m_cardContainer);
            
            // 将卡片区域添加到布局（替代表格位置）
            if (ui->twFlightList->parentWidget() && ui->twFlightList->parentWidget()->layout()) {
                ui->twFlightList->parentWidget()->layout()->addWidget(m_cardScrollArea);
            }
        }
        
        m_cardScrollArea->setVisible(true);
        
        // 刷新卡片视图
        loadFlightsToCards(m_currentFlights);
    } else {
        // 显示表格，隐藏卡片视图
        ui->twFlightList->setVisible(true);
        if (m_cardScrollArea) {
            m_cardScrollArea->setVisible(false);
        }
    }
}

// 加载航班到卡片视图
void FlightManager::loadFlightsToCards(const QList<Flight>& flights)
{
    if (!m_cardContainer) return;
    
    // 清除旧卡片
    QLayout *oldLayout = m_cardContainer->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete oldLayout;
    }
    
    // 创建新的网格布局
    QVBoxLayout *layout = new QVBoxLayout(m_cardContainer);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);
    
    if (flights.isEmpty()) {
        QLabel *emptyLabel = new QLabel("暂无符合条件的航班");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 16px; color: #9E9E9E; padding: 40px;");
        layout->addWidget(emptyLabel);
    } else {
        for (const Flight& flight : flights) {
            QFrame *card = createFlightCard(flight);
            layout->addWidget(card);
        }
    }
    
    layout->addStretch();
}

// 创建单个航班卡片
QFrame* FlightManager::createFlightCard(const Flight& flight)
{
    QFrame *card = new QFrame();
    card->setObjectName("flightCard");
    card->setStyleSheet(R"(
        QFrame#flightCard {
            background-color: #FFFFFF;
            border: 2px solid #E0E0E0;
            border-radius: 12px;
            padding: 0px;
        }
        QFrame#flightCard:hover {
            border-color: #3498DB;
            background-color: #F8FBFF;
        }
    )");
    card->setCursor(Qt::PointingHandCursor);
    
    // 使用QGraphicsDropShadowEffect替代CSS box-shadow
    QGraphicsDropShadowEffect *cardShadow = new QGraphicsDropShadowEffect(card);
    cardShadow->setBlurRadius(12);
    cardShadow->setColor(QColor(0, 0, 0, 30));
    cardShadow->setOffset(0, 2);
    card->setGraphicsEffect(cardShadow);
    
    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(20);
    
    // 左侧：航班信息
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(8);
    
    // 航班号
    QLabel *flightNoLabel = new QLabel(flight.flightNumber());
    flightNoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #42A5F5; background: transparent;");
    
    // 出发-到达
    QHBoxLayout *routeLayout = new QHBoxLayout();
    QLabel *depLabel = new QLabel(flight.departureCity());
    depLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #212121; background: transparent;");
    QLabel *arrowLabel = new QLabel("  ✈️  →  ");
    arrowLabel->setStyleSheet("font-size: 16px; color: #9E9E9E; background: transparent;");
    QLabel *arrLabel = new QLabel(flight.arrivalCity());
    arrLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #212121; background: transparent;");
    routeLayout->addWidget(depLabel);
    routeLayout->addWidget(arrowLabel);
    routeLayout->addWidget(arrLabel);
    routeLayout->addStretch();
    
    // 时间
    QLabel *timeLabel = new QLabel(QString("%1 - %2")
        .arg(flight.departureTime().toString("HH:mm"))
        .arg(flight.arrivalTime().toString("HH:mm")));
    timeLabel->setStyleSheet("font-size: 14px; color: #757575; background: transparent;");
    
    // 日期
    QLabel *dateLabel = new QLabel(flight.departureTime().toString("yyyy年MM月dd日"));
    dateLabel->setStyleSheet("font-size: 13px; color: #9E9E9E; background: transparent;");
    
    leftLayout->addWidget(flightNoLabel);
    leftLayout->addLayout(routeLayout);
    leftLayout->addWidget(timeLabel);
    leftLayout->addWidget(dateLabel);
    
    // 右侧：价格和购票按钮
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightLayout->setSpacing(12);
    
    // 余票
    QLabel *seatsLabel = new QLabel(QString("余票: %1").arg(flight.availableSeats()));
    seatsLabel->setStyleSheet(QString("font-size: 13px; color: %1; background: transparent;")
        .arg(flight.availableSeats() > 0 ? "#4CAF50" : "#F44336"));
    seatsLabel->setAlignment(Qt::AlignRight);
    
    // 价格
    QLabel *priceLabel = new QLabel(QString("¥%1").arg(flight.price(), 0, 'f', 0));
    priceLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #FF5722; background: transparent;");
    priceLabel->setAlignment(Qt::AlignRight);
    
    // 购票按钮
    QPushButton *bookBtn = new QPushButton("立即购票");
    bookBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #42A5F5;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1E88E5;
        }
        QPushButton:disabled {
            background-color: #BDBDBD;
        }
    )");
    bookBtn->setEnabled(flight.availableSeats() > 0);
    bookBtn->setCursor(Qt::PointingHandCursor);
    
    // 连接购票按钮点击事件 - 需要捕获航班号
    QString flightNo = flight.flightNumber();
    connect(bookBtn, &QPushButton::clicked, this, [this, flightNo]() {
        // 选中该航班并触发优化后的购票流程
        m_currentFlightNo = flightNo;
        
        // 在表格中找到对应行并选中（保持兼容）
        for (int row = 0; row < ui->twFlightList->rowCount(); row++) {
            if (ui->twFlightList->item(row, 1) && 
                ui->twFlightList->item(row, 1)->text() == flightNo) {
                ui->twFlightList->selectRow(row);
                break;
            }
        }
        
        // 触发优化后的一站式购票流程
        startBookingProcess();
    });
    
    rightLayout->addWidget(seatsLabel);
    rightLayout->addWidget(priceLabel);
    rightLayout->addWidget(bookBtn);
    
    cardLayout->addLayout(leftLayout, 1);
    cardLayout->addLayout(rightLayout);
    
    return card;
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

    // 检查是否有可用代金券并计算最终应付金额
        // 检查是否有可用代金券并允许用户选择代金券，计算最终应付金额
        double originalPrice = flight.price();
        double voucherValue = 0.0;
        QString appliedVoucherId;
        QString appliedVoucherCode;
        double priceToPay = originalPrice;

        if (!m_currentUserAccount.isEmpty()) {
            MemberSystem& memberSys = MemberSystem::instance();
            QList<Voucher> vouchers = memberSys.getAvailableVouchers(m_currentUserAccount);

            // 准备选择列表，同时维护对应的 voucher id 列表以避免字符串匹配不准确
            QStringList choices;
            QStringList voucherIds;
            choices << "不使用代金券";
            voucherIds << QString(); // 对应“不使用代金券”的占位
            for (const Voucher &v : vouchers) {
                choices << QString("%1  （价值 ¥%2）").arg(v.code).arg(v.value, 0, 'f', 2);
                voucherIds << v.id;
            }

            QString chosen;
            bool ok = false;
            if (!choices.isEmpty()) {
                chosen = QInputDialog::getItem(this, "选择代金券", "请选择要使用的代金券：", choices, 0, false, &ok);
            }

            if (ok && !chosen.isEmpty() && chosen != "不使用代金券") {
                int sel = choices.indexOf(chosen);
                if (sel >= 1 && sel < voucherIds.size()) {
                    appliedVoucherId = voucherIds[sel];
                    // find the voucher by id
                    for (const Voucher &v : vouchers) {
                        if (v.id == appliedVoucherId) {
                            appliedVoucherCode = v.code;
                            voucherValue = v.value;
                            break;
                        }
                    }
                }
            }

            priceToPay = qMax(0.0, originalPrice - voucherValue);

            // 在扣款前，向用户展示最终价格和代金券信息，确认是否继续
            QString finalInfo = QString("原价：¥%1\n").arg(originalPrice, 0, 'f', 2);
            if (!appliedVoucherCode.isEmpty()) {
                finalInfo += QString("已应用代金券：%1（¥%2）\n").arg(appliedVoucherCode).arg(voucherValue, 0, 'f', 2);
            }
            finalInfo += QString("应付金额：¥%1\n\n是否确认支付？").arg(priceToPay, 0, 'f', 2);

            // 使用自定义样式化的 QMessageBox 以保证按钮颜色可见
            QMessageBox finalBox(this);
            finalBox.setWindowTitle(QString::fromUtf8("确认支付"));
            finalBox.setText(finalInfo);
            finalBox.setIcon(QMessageBox::Question);
            finalBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            QPushButton *yesBtn = qobject_cast<QPushButton*>(finalBox.button(QMessageBox::Yes));
            QPushButton *noBtn = qobject_cast<QPushButton*>(finalBox.button(QMessageBox::No));
            if (yesBtn) yesBtn->setText(QString::fromUtf8("确认"));
            if (noBtn) noBtn->setText(QString::fromUtf8("取消"));
            finalBox.setStyleSheet(R"(
                QMessageBox { background-color: white; color: #222222; font-family: "微软雅黑"; font-size: 14px; }
                QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 8px; padding: 8px 20px; font-size: 14px; }
                QPushButton#qt_msgbox_no { background-color: #666666; }
                QPushButton:hover { opacity: 0.95; }
            )");
            finalBox.layout()->activate();
            if (finalBox.exec() != QMessageBox::Yes) {
                return; // 用户取消
            }

            MemberInfo memberInfo = memberSys.getMemberInfo(m_currentUserAccount);
            if (memberInfo.balance < priceToPay) {
                QMessageBox::critical(this, "余额不足",
                    QString("您的飞机币余额不足！\n当前余额：¥%1\n应付金额：¥%2\n(含已应用代金券 ¥%3)")
                    .arg(memberInfo.balance, 0, 'f', 2)
                    .arg(priceToPay, 0, 'f', 2)
                    .arg(voucherValue, 0, 'f', 2));
                return;
            }

            // 扣除飞机币（仅扣除应付部分）
            if (priceToPay > 0.0) {
                if (!memberSys.deductBalance(m_currentUserAccount, priceToPay,
                    QString("购买航班 %1 座位 %2").arg(flight.flightNumber()).arg(m_selectedSeat))) {
                    QMessageBox::critical(this, "失败", "扣除飞机币失败，请重试！");
                    return;
                }
                qDebug() << "飞机币扣除成功：" << priceToPay;
            } else {
                qDebug() << "使用代金券全额抵扣，未扣除飞机币";
            }
        }

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
    // 订单金额记录为实际支付金额（含代金券抵扣后）
    newOrder.setPrice(priceToPay);
    newOrder.setStatus("已支付");

    // 写入数据库
    if (!DBManager::instance().addOrder(newOrder)) {
        updatedFlight.setAvailableSeats(flight.availableSeats());
        DBManager::instance().updateFlight(updatedFlight);

        // 恢复飞机币（退款应付金额）
        if (!m_currentUserAccount.isEmpty() && priceToPay > 0.0) {
            MemberSystem::instance().addBalance(m_currentUserAccount, priceToPay, "订单创建失败，退款");
        }

        QMessageBox::critical(this, "失败", "订单创建失败，座位已恢复！");
        return;
    }
    qDebug() << "订单创建成功：" << newOrder.orderNumber();
    
    // 【新增】设置订单中的用户账号
    newOrder.setUserId(m_currentUserAccount);
    
    // 标记座位为已售
    if (!DBManager::instance().markSeatAsSold(flight.flightNumber(), m_selectedSeat)) {
        qWarning() << "座位标记失败，但订单已创建";
    }
    
    // 增加飞行里程（如果有当前用户）
    if (!m_currentUserAccount.isEmpty()) {
        double distance = calculateFlightDistance(flight.departureCity(), flight.arrivalCity());
        MemberSystem::instance().addMileage(m_currentUserAccount, distance);
        qDebug() << "飞行里程已增加：" << distance << "公里";
    }

    // 如果使用了代金券，标记为已用并写入 redemptions 记录（type='use'）
        if (!appliedVoucherId.isEmpty() && !m_currentUserAccount.isEmpty()) {
            // 使用 MemberSystem 的封装方法标记为已用（会写入 redemptions）
            if (!MemberSystem::instance().markVoucherUsed(appliedVoucherId, newOrder.orderNumber())) {
                qWarning() << "标记代金券已用失败：" << appliedVoucherId;
            }
        }

        // 购票成功后增加积分（按实际支付金额累积，1 元 = 1 积分）
        if (!m_currentUserAccount.isEmpty()) {
            int pointsEarned = static_cast<int>(qFloor(priceToPay));
            if (pointsEarned > 0) {
                if (MemberSystem::instance().addPoints(m_currentUserAccount, pointsEarned)) {
                    qDebug() << "已增加积分：" << pointsEarned << "给用户" << m_currentUserAccount;
                } else {
                    qWarning() << "增加积分失败";
                }
            }
        }
        
        // 【新增】发送购票成功邮件提醒
        if (!m_currentUserAccount.isEmpty()) {
            UserProfile userProfile = DBManager::instance().loadUserProfile(m_currentUserAccount);
            // 即使没有邮箱也发送系统内部提醒
            EmailReminder::instance().sendTicketBookedReminder(
                userProfile.email,
                userProfile.nickname.isEmpty() ? m_currentUserAccount : userProfile.nickname,
                m_currentUserAccount,
                flight.flightNumber(),
                flight.departureCity(),
                flight.arrivalCity(),
                flight.departureTime().toString("yyyy-MM-dd HH:mm"),
                newOrder.seatNumber(),
                priceToPay
            );
        }

    // 重置状态
    m_selectedSeat.clear();
    m_currentFlightNo.clear();

    // 刷新+发送信号
    onRefreshClicked();
    emit orderCreated();

    // 购票成功弹窗 — 显示原价与实际支付（若使用代金券则会不同）
    QString bookSuccessText = QString("购票成功！\n\n订单号：%1\n航班号：%2\n出发城市：%3\n到达城市：%4\n出发时间：%5\n座位号：%6\n原价：¥%7\n实付：¥%8")
                                  .arg(newOrder.orderNumber())
                                  .arg(flight.flightNumber())
                                  .arg(flight.departureCity())
                                  .arg(flight.arrivalCity())
                                  .arg(flight.departureTime().toString("yyyy-MM-dd HH:mm"))
                                  .arg(newOrder.seatNumber())
                                  .arg(flight.price(), 0, 'f', 2)
                                  .arg(priceToPay, 0, 'f', 2);

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

// ========== 优化后的一站式购票流程（支持返回上一步） ==========
void FlightManager::startBookingProcess()
{
    // 步骤1：获取选中的航班
    Flight flight = getSelectedFlight();
    if (flight.flightNumber().isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择航班");
        return;
    }

    if (flight.availableSeats() <= 0) {
        QMessageBox::warning(this, "提示", "该航班已无可用座位！");
        return;
    }

    // 准备座位布局数据
    FlightInfo info;
    info.flightNumber = flight.flightNumber();
    info.departureCity = flight.departureCity();
    info.arrivalCity = flight.arrivalCity();
    info.dateTime = flight.departureTime().toString("yyyy-MM-dd HH:mm");

    int totalSeats = flight.totalSeats();
    int availableSeats = flight.availableSeats();
    int soldSeats = totalSeats - availableSeats;
    int seatsPerRow = 6;
    int totalRows = (totalSeats + seatsPerRow - 1) / seatsPerRow;
    
    QVector<SeatData> seats;
    QString seatCols = "ABCDEF";
    QSet<QString> soldSeatIds;
    int soldCount = 0;
    uint seed = qHash(flight.flightNumber());
    
    while (soldCount < soldSeats) {
        int randRow = (seed % totalRows) + 1;
        int randCol = (seed / totalRows) % seatsPerRow;
        seed = seed * 1103515245 + 12345;
        
        QString seatId = QString::number(randRow) + seatCols[randCol];
        if (!soldSeatIds.contains(seatId)) {
            soldSeatIds.insert(seatId);
            soldCount++;
        }
        if (soldCount >= totalSeats) break;
    }
    
    for (int row = 1; row <= totalRows; row++) {
        for (int col = 0; col < seatsPerRow; col++) {
            QString seatId = QString::number(row) + seatCols[col];
            SeatData seat;
            seat.seatId = seatId;
            seat.state = soldSeatIds.contains(seatId) ? Sold : Available;
            seat.isWindow = (col == 0 || col == 5);
            seat.isAisle = (col == 2 || col == 3);
            seats.append(seat);
        }
    }
    info.allSeats = seats;

    // 购票流程变量
    QString selectedSeat;
    double originalPrice = flight.price();
    QString appliedVoucherId;
    QString appliedVoucherCode;
    double voucherValue = 0.0;
    double finalPrice = originalPrice;

    int currentStep = 1;  // 1=选座, 2=代金券, 3=确认订单

    while (currentStep >= 1 && currentStep <= 3) {
        if (currentStep == 1) {
            // 步骤1：选座对话框
            SeatDialog seatDialog(info, this);
            if (seatDialog.exec() != QDialog::Accepted) {
                return; // 用户取消，退出整个流程
            }
            selectedSeat = seatDialog.getSelectedSeatId();
            if (selectedSeat.isEmpty()) {
                QMessageBox::warning(this, "提示", "请选择座位！");
                continue;
            }
            currentStep = 2;
        }
        else if (currentStep == 2) {
            // 步骤2：代金券选择对话框
            if (!m_currentUserAccount.isEmpty()) {
                VoucherDialog voucherDialog(m_currentUserAccount, originalPrice, this);
                if (voucherDialog.exec() == QDialog::Accepted) {
                    appliedVoucherId = voucherDialog.getSelectedVoucherId();
                    appliedVoucherCode = voucherDialog.getSelectedVoucherCode();
                    voucherValue = voucherDialog.getVoucherValue();
                    finalPrice = voucherDialog.getFinalPrice();
                    currentStep = 3;
                } else {
                    // 用户点击返回
                    currentStep = 1;
                }
            } else {
                currentStep = 3;
            }
        }
        else if (currentStep == 3) {
            // 步骤3：订单确认对话框
            MemberInfo memberInfo;
            if (!m_currentUserAccount.isEmpty()) {
                memberInfo = MemberSystem::instance().getMemberInfo(m_currentUserAccount);
            }

            OrderConfirmDialog::OrderInfo orderInfo;
            orderInfo.flightNumber = flight.flightNumber();
            orderInfo.departureCity = flight.departureCity();
            orderInfo.arrivalCity = flight.arrivalCity();
            orderInfo.departureTime = flight.departureTime().toString("yyyy-MM-dd HH:mm");
            orderInfo.seatNumber = selectedSeat;
            orderInfo.originalPrice = originalPrice;
            orderInfo.voucherCode = appliedVoucherCode;
            orderInfo.voucherValue = voucherValue;
            orderInfo.finalPrice = finalPrice;
            orderInfo.userBalance = memberInfo.balance;

            OrderConfirmDialog confirmDialog(orderInfo, this);
            if (confirmDialog.exec() == QDialog::Accepted) {
                // 执行购票逻辑
                executeBooking(flight, selectedSeat, appliedVoucherId, appliedVoucherCode, voucherValue, finalPrice, memberInfo);
                return;
            } else {
                // 用户点击返回
                currentStep = 2;
            }
        }
    }
}

// 执行购票逻辑（从 startBookingProcess 中提取）
void FlightManager::executeBooking(const Flight& flight, const QString& selectedSeat,
                                    const QString& appliedVoucherId, const QString& /*appliedVoucherCode*/,
                                    double /*voucherValue*/, double finalPrice, const MemberInfo& memberInfo)
{
    // 检查余额
    if (!m_currentUserAccount.isEmpty() && memberInfo.balance < finalPrice) {
        QMessageBox::critical(this, "余额不足",
            QString("您的飞机币余额不足！\n当前余额：¥%1\n应付金额：¥%2")
            .arg(memberInfo.balance, 0, 'f', 2)
            .arg(finalPrice, 0, 'f', 2));
        return;
    }

    // 扣除飞机币
    if (!m_currentUserAccount.isEmpty() && finalPrice > 0.0) {
        if (!MemberSystem::instance().deductBalance(m_currentUserAccount, finalPrice,
            QString("购买航班 %1 座位 %2").arg(flight.flightNumber()).arg(selectedSeat))) {
            QMessageBox::critical(this, "失败", "扣除飞机币失败，请重试！");
            return;
        }
    }

    // 更新可用座位
    Flight updatedFlight = flight;
    updatedFlight.setAvailableSeats(flight.availableSeats() - 1);
    if (!DBManager::instance().updateFlight(updatedFlight)) {
        // 恢复飞机币
        if (!m_currentUserAccount.isEmpty() && finalPrice > 0.0) {
            MemberSystem::instance().addBalance(m_currentUserAccount, finalPrice, "座位更新失败，退款");
        }
        QMessageBox::critical(this, "失败", "座位更新失败，请重试！");
        return;
    }

    // 生成订单
    Order newOrder;
    newOrder.setOrderNumber(generateOrderNumber());
    newOrder.setFlightNumber(flight.flightNumber());
    newOrder.setDepartureCity(flight.departureCity());
    newOrder.setArrivalCity(flight.arrivalCity());
    newOrder.setDepartTime(flight.departureTime());
    newOrder.setSeatNumber(selectedSeat);
    newOrder.setPrice(finalPrice);
    newOrder.setStatus("已支付");
    newOrder.setUserId(m_currentUserAccount);

    if (!DBManager::instance().addOrder(newOrder)) {
        // 恢复座位和飞机币
        updatedFlight.setAvailableSeats(flight.availableSeats());
        DBManager::instance().updateFlight(updatedFlight);
        if (!m_currentUserAccount.isEmpty() && finalPrice > 0.0) {
            MemberSystem::instance().addBalance(m_currentUserAccount, finalPrice, "订单创建失败，退款");
        }
        QMessageBox::critical(this, "失败", "订单创建失败！");
        return;
    }

    // 标记座位为已售
    DBManager::instance().markSeatAsSold(flight.flightNumber(), selectedSeat);

    // 标记代金券已用
    if (!appliedVoucherId.isEmpty() && !m_currentUserAccount.isEmpty()) {
        MemberSystem::instance().markVoucherUsed(appliedVoucherId, newOrder.orderNumber());
    }

    // 增加飞行里程
    if (!m_currentUserAccount.isEmpty()) {
        double distance = calculateFlightDistance(flight.departureCity(), flight.arrivalCity());
        MemberSystem::instance().addMileage(m_currentUserAccount, distance);
    }

    // 增加积分
    if (!m_currentUserAccount.isEmpty()) {
        int pointsEarned = static_cast<int>(qFloor(finalPrice));
        if (pointsEarned > 0) {
            MemberSystem::instance().addPoints(m_currentUserAccount, pointsEarned);
        }
    }

    // 发送购票成功邮件提醒
    if (!m_currentUserAccount.isEmpty()) {
        UserProfile userProfile = DBManager::instance().loadUserProfile(m_currentUserAccount);
        EmailReminder::instance().sendTicketBookedReminder(
            userProfile.email,
            userProfile.nickname.isEmpty() ? m_currentUserAccount : userProfile.nickname,
            m_currentUserAccount,
            flight.flightNumber(),
            flight.departureCity(),
            flight.arrivalCity(),
            flight.departureTime().toString("yyyy-MM-dd HH:mm"),
            selectedSeat,
            finalPrice
        );
    }

    // 更新座位变量
    m_selectedSeat = selectedSeat;

    // 刷新界面
    onRefreshClicked();

    // 发送订单创建信号
    emit orderCreated();

    // 显示精美的成功提示对话框
    QDialog *successDialog = new QDialog(this);
    successDialog->setWindowTitle("购票成功");
    successDialog->setMinimumSize(450, 420);
    successDialog->setStyleSheet(R"(
        QDialog {
            background-color: #F0FDF4;
        }
        QLabel {
            background-color: transparent;
        }
    )");

    QVBoxLayout *layout = new QVBoxLayout(successDialog);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 30, 40, 30);

    // 成功图标
    QLabel *iconLabel = new QLabel("✅", successDialog);
    iconLabel->setStyleSheet("font-size: 50px; background: transparent;");
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    // 标题
    QLabel *titleLabel = new QLabel("购票成功！", successDialog);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #166534; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    // 信息区域（使用卡片与栅格布局，避免文本框线）
    QFrame *infoCard = new QFrame(successDialog);
    infoCard->setStyleSheet(R"(
        QFrame {
            background-color: #FFFFFF;
            border: none;
            border-radius: 12px;
        }
    )");
    QGridLayout *infoGrid = new QGridLayout(infoCard);
    infoGrid->setContentsMargins(18, 14, 18, 14);
    infoGrid->setHorizontalSpacing(8);
    infoGrid->setVerticalSpacing(10);

    auto makeRow = [&](int row, const QString &label, const QString &value, const QString &valueColor, int valueSize, bool bold = true) {
        QLabel *l = new QLabel(label, infoCard);
        l->setStyleSheet("font-size: 14px; color: #6B7280;");
        QLabel *v = new QLabel(value, infoCard);
        v->setStyleSheet(QString("font-size: %1px; color: %2; font-weight: %3;")
                         .arg(valueSize)
                         .arg(valueColor)
                         .arg(bold ? "600" : "400"));
        infoGrid->addWidget(l, row, 0, Qt::AlignLeft);
        infoGrid->addWidget(v, row, 1, Qt::AlignRight);
    };

    makeRow(0, "航班号：", flight.flightNumber(), "#2563EB", 16);
    makeRow(1, "路线：", QString("%1 → %2").arg(flight.departureCity(), flight.arrivalCity()), "#111827", 16, true);
    makeRow(2, "座位号：", selectedSeat, "#0EA5E9", 16);
    makeRow(3, "支付金额：", QString("¥%1").arg(finalPrice, 0, 'f', 2), "#EF4444", 18);

    layout->addWidget(infoCard);

    // 祝福语
    QLabel *wishLabel = new QLabel("🛫 祝您旅途愉快！", successDialog);
    wishLabel->setStyleSheet("font-size: 15px; color: #059669; background: transparent;");
    wishLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(wishLabel);

    layout->addStretch();

    // 确认按钮
    QPushButton *okBtn = new QPushButton("确认", successDialog);
    okBtn->setFixedSize(150, 45);
    okBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #10B981;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #059669;
        }
    )");
    okBtn->setCursor(Qt::PointingHandCursor);
    connect(okBtn, &QPushButton::clicked, successDialog, &QDialog::accept);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addStretch();
    layout->addLayout(btnLayout);

    successDialog->exec();
    delete successDialog;
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

    // ========== 动态生成座位布局 ==========
    // 根据航班的总座位数动态生成座位
    int totalSeats = flight.totalSeats();
    int availableSeats = flight.availableSeats();
    int soldSeats = totalSeats - availableSeats;
    
    // 计算座位布局
    // 假设每排6个座位 (A B C | D E F)
    int seatsPerRow = 6;
    int totalRows = (totalSeats + seatsPerRow - 1) / seatsPerRow;
    
    QVector<SeatData> seats;
    QString seatCols = "ABCDEF";
    
    // 随机分布已售座位（用于演示）
    QSet<QString> soldSeatIds;
    int soldCount = 0;
    
    // 使用航班号的哈希值作为随机种子，确保同一航班每次座位分布相同
    uint seed = qHash(flight.flightNumber());
    
    // 生成需要标记为已售的座位位置
    while (soldCount < soldSeats) {
        int randRow = (seed % totalRows) + 1;
        int randCol = (seed / totalRows) % seatsPerRow;
        seed = seed * 1103515245 + 12345; // 简单的线性同余生成器
        
        QString seatId = QString::number(randRow) + seatCols[randCol];
        if (!soldSeatIds.contains(seatId)) {
            soldSeatIds.insert(seatId);
            soldCount++;
        }
        
        // 防止无限循环
        if (soldCount >= totalSeats) break;
    }
    
    // 生成所有座位
    for (int row = 1; row <= totalRows; row++) {
        for (int col = 0; col < seatsPerRow; col++) {
            QString seatId = QString::number(row) + seatCols[col];
            SeatData seat;
            seat.seatId = seatId;
            seat.state = soldSeatIds.contains(seatId) ? Sold : Available;
            seat.isWindow = (col == 0 || col == 5); // A和F是靠窗
            seat.isAisle = (col == 2 || col == 3);  // C和D是靠过道
            seats.append(seat);
        }
    }
    
    info.allSeats = seats;
    // ========== 动态座位布局结束 ==========

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

    // 管理员按钮状态控制
    if (m_isAdminMode) {
        ui->btnEdit->setEnabled(hasSelection);
        ui->btnDelete->setEnabled(hasSelection);
    }
}

// 现代化样式应用
void FlightManager::applyModernStyle()
{
    // 整体背景
    this->setStyleSheet(R"(
        QWidget {
            background-color: #f5f7fa;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
        }
    )");
    
    // 搜索区域输入框样式
    QString inputStyle = R"(
        QLineEdit, QDateEdit {
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 13px;
            background-color: #fff;
        }
        QLineEdit:focus, QDateEdit:focus {
            border-color: #4caf50;
        }
    )";
    
    if (ui->leDeparture) {
        ui->leDeparture->setStyleSheet(inputStyle);
        ui->leDeparture->setPlaceholderText("出发城市");
    }
    if (ui->leArrival) {
        ui->leArrival->setStyleSheet(inputStyle);
        ui->leArrival->setPlaceholderText("到达城市");
    }
    if (ui->deDate) {
        ui->deDate->setStyleSheet(inputStyle);
    }
    
    // 主要操作按钮样式
    QString primaryBtnStyle = R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #43a047;
        }
        QPushButton:pressed {
            background-color: #388e3c;
        }
        QPushButton:disabled {
            background-color: #bdbdbd;
        }
    )";
    
    QString secondaryBtnStyle = R"(
        QPushButton {
            background-color: #f5f5f5;
            color: #333;
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
        }
        QPushButton:hover {
            background-color: #e0e0e0;
            border-color: #ccc;
        }
    )";
    
    QString accentBtnStyle = R"(
        QPushButton {
            background-color: #1976d2;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1565c0;
        }
        QPushButton:disabled {
            background-color: #bdbdbd;
        }
    )";
    
    QString dangerBtnStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #EF4444, stop:1 #DC2626);
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #F87171, stop:1 #EF4444);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #DC2626, stop:1 #B91C1C);
        }
    )";
    
    // 应用按钮样式
    if (ui->btnSearch) {
        ui->btnSearch->setStyleSheet(primaryBtnStyle);
        ui->btnSearch->setCursor(Qt::PointingHandCursor);
        ui->btnSearch->setText("🔍 搜索航班");
    }
    if (ui->btnRefresh) {
        ui->btnRefresh->setStyleSheet(secondaryBtnStyle);
        ui->btnRefresh->setCursor(Qt::PointingHandCursor);
        ui->btnRefresh->setText("🔄 刷新");
    }
    if (ui->btnAdd) {
        ui->btnAdd->setStyleSheet(primaryBtnStyle);
        ui->btnAdd->setCursor(Qt::PointingHandCursor);
    }
    if (ui->btnEdit) {
        ui->btnEdit->setStyleSheet(accentBtnStyle);
        ui->btnEdit->setCursor(Qt::PointingHandCursor);
    }
    if (ui->btnDelete) {
        ui->btnDelete->setStyleSheet(dangerBtnStyle);
        ui->btnDelete->setCursor(Qt::PointingHandCursor);
    }
}
