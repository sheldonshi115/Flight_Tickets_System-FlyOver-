// ticketbooking.cpp
#include "ticketbooking.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDate>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QColor>
#include <QSize>

TicketBookingWidget::TicketBookingWidget(QWidget *parent)
    : QWidget(parent), m_selectedFlightIndex(-1)
{
    setupUI();
    loadSampleFlights();
}

TicketBookingWidget::~TicketBookingWidget()
{
}

void TicketBookingWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    // 顶部标题
    QLabel *titleLabel = new QLabel("🛫 购票系统");
    titleLabel->setStyleSheet(R"(
        font-size: 28px;
        font-weight: bold;
        color: #1F2937;
        margin-bottom: 12px;
        padding: 0;
    )");
    mainLayout->addWidget(titleLabel);

    // 搜索面板
    QGroupBox *searchGroup = setupSearchPanel();
    mainLayout->addWidget(searchGroup);

    // 结果面板
    QGroupBox *resultGroup = setupFlightResults();
    mainLayout->addWidget(resultGroup, 1);
}

QGroupBox* TicketBookingWidget::setupSearchPanel()
{
    QGroupBox *searchGroup = new QGroupBox("✈️ 搜索航班");
    searchGroup->setStyleSheet(R"(
        QGroupBox {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 16px;
            padding: 20px;
            margin-top: 12px;
            font-size: 16px;
            font-weight: 600;
            color: #1F2937;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #3B82F6;
        }
    )");
    
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
    searchLayout->setSpacing(16);
    searchLayout->setContentsMargins(16, 24, 16, 16);

    // 第一行：出发地和目的地
    QHBoxLayout *cityLayout = new QHBoxLayout();
    QLabel *fromLabel = new QLabel("出发地");
    fromLabel->setStyleSheet("font-weight: 600; color: #374151; font-size: 14px;");
    m_fromCity = new QComboBox();
    m_fromCity->addItems({"广州", "北京", "上海", "深圳", "成都", "西安"});
    m_fromCity->setStyleSheet(R"(
        QComboBox {
            background-color: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 14px;
            color: #1F2937;
        }
        QComboBox:hover {
            border-color: #60A5FA;
            background-color: #FFFFFF;
        }
        QComboBox:focus {
            border-color: #3B82F6;
            background-color: #FFFFFF;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #6B7280;
            margin-right: 8px;
        }
    )");

    QLabel *toLabel = new QLabel("目的地");
    toLabel->setStyleSheet("font-weight: 600; color: #374151; font-size: 14px;");
    m_toCity = new QComboBox();
    m_toCity->addItems({"上海", "北京", "广州", "深圳", "杭州", "南京"});
    m_toCity->setStyleSheet(m_fromCity->styleSheet());

    cityLayout->addWidget(fromLabel);
    cityLayout->addWidget(m_fromCity, 1);
    cityLayout->addWidget(toLabel);
    cityLayout->addWidget(m_toCity, 1);
    searchLayout->addLayout(cityLayout);

    // 第二行：日期选择
    QHBoxLayout *dateLayout = new QHBoxLayout();
    QLabel *dateLabel = new QLabel("出发日期");
    dateLabel->setStyleSheet("font-weight: 600; color: #374151; font-size: 14px;");
    m_departDate = new QDateEdit();
    m_departDate->setDate(QDate::currentDate());
    m_departDate->setStyleSheet(R"(
        QDateEdit {
            background-color: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 14px;
            color: #1F2937;
        }
        QDateEdit:hover {
            border-color: #60A5FA;
            background-color: #FFFFFF;
        }
        QDateEdit:focus {
            border-color: #3B82F6;
            background-color: #FFFFFF;
        }
    )");

    QLabel *returnLabel = new QLabel("返回日期");
    returnLabel->setStyleSheet("font-weight: 600; color: #374151; font-size: 14px;");
    m_returnDate = new QDateEdit();
    m_returnDate->setDate(QDate::currentDate().addDays(7));
    m_returnDate->setStyleSheet(m_departDate->styleSheet());

    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(m_departDate, 1);
    dateLayout->addWidget(returnLabel);
    dateLayout->addWidget(m_returnDate, 1);
    searchLayout->addLayout(dateLayout);

    // 搜索按钮
    m_searchBtn = new QPushButton("🔍 搜索航班");
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    m_searchBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3B82F6, stop:1 #60A5FA);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 24px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563EB, stop:1 #3B82F6);
        }
        QPushButton:pressed {
            background: #1D4ED8;
        }
    )");
    searchLayout->addWidget(m_searchBtn);
    connect(m_searchBtn, &QPushButton::clicked, this, &TicketBookingWidget::onSearchFlights);

    // 排序方式
    QLabel *sortLabel = new QLabel("排序方式");
    sortLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 12px; font-size: 14px;");
    searchLayout->addWidget(sortLabel);

    m_filterList = new QListWidget();
    m_filterList->setMaximumHeight(50);
    m_filterList->addItem("🔄 推荐排序");
    m_filterList->addItem("💰 价格最低");
    m_filterList->addItem("⏰ 出发最早");
    searchLayout->addWidget(m_filterList);

    return searchGroup;
}

QGroupBox* TicketBookingWidget::setupFlightResults()
{
    QGroupBox *resultGroup = new QGroupBox("📋 航班列表");
    resultGroup->setStyleSheet(R"(
        QGroupBox {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 16px;
            padding: 20px;
            margin-top: 12px;
            font-size: 16px;
            font-weight: 600;
            color: #1F2937;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            color: #3B82F6;
        }
    )");
    
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    resultLayout->setSpacing(16);
    resultLayout->setContentsMargins(16, 24, 16, 16);

    // 航班列表
    m_flightList = new QListWidget();
    m_flightList->setSpacing(12);
    m_flightList->setStyleSheet(R"(
        QListWidget {
            background-color: #F9FAFB;
            border: 2px solid #E5E7EB;
            border-radius: 12px;
            padding: 8px;
        }
        QListWidget::item {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 10px;
            padding: 16px;
            margin: 4px;
        }
        QListWidget::item:hover {
            background-color: #EFF6FF;
            border-color: #93C5FD;
        }
        QListWidget::item:selected {
            background-color: #DBEAFE;
            border-color: #3B82F6;
        }
    )");
    resultLayout->addWidget(m_flightList, 1);

    // 选中航班信息
    QLabel *detailLabel = new QLabel("航班详情");
    detailLabel->setStyleSheet("font-weight: 600; color: #374151; margin-top: 16px; font-size: 14px;");
    resultLayout->addWidget(detailLabel);

    m_selectedFlightInfo = new QLabel("选择左侧航班查看详情");
    m_selectedFlightInfo->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #FEFCE8, stop:1 #FEF3C7);
            padding: 20px;
            border-radius: 12px;
            border-left: 4px solid #F59E0B;
            min-height: 100px;
            color: #78350F;
            font-size: 14px;
        }
    )");
    m_selectedFlightInfo->setWordWrap(true);
    resultLayout->addWidget(m_selectedFlightInfo);

    // 预定按钮
    m_bookBtn = new QPushButton("✅ 确认预定");
    m_bookBtn->setCursor(Qt::PointingHandCursor);
    m_bookBtn->setEnabled(false);
    m_bookBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #10B981, stop:1 #34D399);
            color: white;
            border: none;
            border-radius: 12px;
            padding: 14px 24px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover:enabled {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #059669, stop:1 #10B981);
        }
        QPushButton:pressed:enabled {
            background: #047857;
        }
        QPushButton:disabled {
            background-color: #E5E7EB;
            color: #9CA3AF;
        }
    )");
    resultLayout->addWidget(m_bookBtn);
    resultLayout->addWidget(m_bookBtn);

    connect(m_flightList, &QListWidget::itemClicked, this, &TicketBookingWidget::onFlightListItemClicked);
    connect(m_bookBtn, &QPushButton::clicked, this, &TicketBookingWidget::onBookTicket);

    return resultGroup;
}

void TicketBookingWidget::loadSampleFlights()
{
    m_searchResults = {
        {"CA101", "国航", "白云", "虹桥", "21:35", "23:50", 674, 321, "中"},
        {"CA1866", "国航", "白云", "浦东", "21:40", "23:59", 746, 350, "大"},
        {"HO1852", "吉祥", "白云", "虹桥", "21:25", "23:25", 815, 787, "大"},
        {"CZ3832", "南航", "白云", "浦东", "22:15", "00:45", 980, 321, "中"},
        {"MU5672", "东航", "白云", "浦东", "18:45", "21:00", 1045, 280, "中"},
    };

    loadFlightResults();
}

void TicketBookingWidget::loadFlightResults()
{
    m_flightList->clear();

    for (int i = 0; i < m_searchResults.size(); ++i) {
        const FlightInfo& flight = m_searchResults[i];

        QString itemText = QString(
            "✈️ %1 | %2\n"
            "%3 → %4\n"
            "%5 ➜ %6 | 空客%7 | ¥%8"
        ).arg(flight.flightNumber, flight.airline, 
              flight.departure, flight.arrival,
              flight.departTime, flight.arriveTime,
              flight.type, QString::number((int)flight.price));

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        
        // 设置项目高度
        item->setSizeHint(QSize(0, 80));
        
        // 设置样式
        item->setBackground(QColor(255, 255, 255));
        
        m_flightList->addItem(item);
    }
}

void TicketBookingWidget::onSearchFlights()
{
    // 根据选择的城市和日期搜索
    loadFlightResults();
}

void TicketBookingWidget::onFlightListItemClicked()
{
    QListWidgetItem *item = m_flightList->currentItem();
    if (item) {
        m_selectedFlightIndex = item->data(Qt::UserRole).toInt();
        const FlightInfo& flight = m_searchResults[m_selectedFlightIndex];

        QString info = QString(
            "<table style='width: 100%;'>"
            "<tr><td><b>航班号</b></td><td>%1 <span style='color: #999;'>(%2)</span></td></tr>"
            "<tr><td><b>出发</b></td><td>%3 <span style='color: #ff9500;'>%4</span></td></tr>"
            "<tr><td><b>到达</b></td><td>%5 <span style='color: #ff9500;'>%6</span></td></tr>"
            "<tr><td><b>机型</b></td><td>空客%7 (%8座) | <b style='color: #4caf50;'>¥%9</b></td></tr>"
            "</table>"
        ).arg(flight.flightNumber, flight.airline,
              flight.departure, flight.departTime,
              flight.arrival, flight.arriveTime,
              flight.type, QString::number(flight.passengers),
              QString::number((int)flight.price));

        m_selectedFlightInfo->setText(info);
        m_bookBtn->setEnabled(true);
    }
}

void TicketBookingWidget::onBookTicket()
{
    if (m_selectedFlightIndex >= 0) {
        // 跳转到订单确认页面
    }
}
