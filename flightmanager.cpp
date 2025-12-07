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
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSet>
#include <QGraphicsDropShadowEffect>

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

    // 清空原有数据
    ui->twFlightList->setRowCount(0);

    // 填充新数据
    for (const auto& flight : flights) {
        int row = ui->twFlightList->rowCount();
        ui->twFlightList->insertRow(row);

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
        
        // 票价 - 右对齐，保癹2位小数
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
        
        // 座位不足时显示红色警告
        if (flight.availableSeats() < 10) {
            availSeatsItem->setForeground(QBrush(QColor("#DC2626"))); // 红色
            availSeatsItem->setFont(QFont("Microsoft YaHei UI", -1, QFont::Bold));
        } else if (flight.availableSeats() < 50) {
            availSeatsItem->setForeground(QBrush(QColor("#F59E0B"))); // 橙色
        } else {
            availSeatsItem->setForeground(QBrush(QColor("#10B981"))); // 绿色
        }
        
        ui->twFlightList->setItem(row, 8, availSeatsItem);
    }

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
        // 选中该航班并触发购票流程
        m_currentFlightNo = flightNo;
        
        // 在表格中找到对应行并选中（保持兼容）
        for (int row = 0; row < ui->twFlightList->rowCount(); row++) {
            if (ui->twFlightList->item(row, 1) && 
                ui->twFlightList->item(row, 1)->text() == flightNo) {
                ui->twFlightList->selectRow(row);
                break;
            }
        }
        
        // 触发选座
        m_isManualClick = true;
        onSelectSeatClicked();
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
    if (ui->btnSeat) {
        ui->btnSeat->setStyleSheet(accentBtnStyle);
        ui->btnSeat->setCursor(Qt::PointingHandCursor);
        ui->btnSeat->setText("💺 选择座位");
    }
    if (ui->btnBook) {
        ui->btnBook->setStyleSheet(primaryBtnStyle);
        ui->btnBook->setCursor(Qt::PointingHandCursor);
        ui->btnBook->setText("🎫 确认购票");
    }
    if (ui->btnExit) {
        ui->btnExit->setStyleSheet(secondaryBtnStyle);
        ui->btnExit->setCursor(Qt::PointingHandCursor);
        ui->btnExit->setText("← 返回");
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
