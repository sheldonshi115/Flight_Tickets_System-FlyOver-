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
    mainLayout->setSpacing(16);

    // 顶部标题
    QLabel *titleLabel = new QLabel("🛫 购票系统");
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #333; margin-bottom: 8px;");
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
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
    searchLayout->setSpacing(12);

    // 第一行：出发地和目的地
    QHBoxLayout *cityLayout = new QHBoxLayout();
    QLabel *fromLabel = new QLabel("出发地");
    fromLabel->setStyleSheet("font-weight: bold; color: #555;");
    m_fromCity = new QComboBox();
    m_fromCity->addItems({"广州", "北京", "上海", "深圳", "成都", "西安"});

    QLabel *toLabel = new QLabel("目的地");
    toLabel->setStyleSheet("font-weight: bold; color: #555;");
    m_toCity = new QComboBox();
    m_toCity->addItems({"上海", "北京", "广州", "深圳", "杭州", "南京"});

    cityLayout->addWidget(fromLabel);
    cityLayout->addWidget(m_fromCity, 1);
    cityLayout->addWidget(toLabel);
    cityLayout->addWidget(m_toCity, 1);
    searchLayout->addLayout(cityLayout);

    // 第二行：日期选择
    QHBoxLayout *dateLayout = new QHBoxLayout();
    QLabel *dateLabel = new QLabel("出发日期");
    dateLabel->setStyleSheet("font-weight: bold; color: #555;");
    m_departDate = new QDateEdit();
    m_departDate->setDate(QDate::currentDate());

    QLabel *returnLabel = new QLabel("返回日期");
    returnLabel->setStyleSheet("font-weight: bold; color: #555;");
    m_returnDate = new QDateEdit();
    m_returnDate->setDate(QDate::currentDate().addDays(7));

    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(m_departDate, 1);
    dateLayout->addWidget(returnLabel);
    dateLayout->addWidget(m_returnDate, 1);
    searchLayout->addLayout(dateLayout);

    // 搜索按钮
    m_searchBtn = new QPushButton("🔍 搜索航班");
    m_searchBtn->setCursor(Qt::PointingHandCursor);
    searchLayout->addWidget(m_searchBtn);
    connect(m_searchBtn, &QPushButton::clicked, this, &TicketBookingWidget::onSearchFlights);

    // 排序方式
    QLabel *sortLabel = new QLabel("排序方式");
    sortLabel->setStyleSheet("font-weight: bold; color: #555; margin-top: 8px;");
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
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
    resultLayout->setSpacing(12);

    // 航班列表
    m_flightList = new QListWidget();
    m_flightList->setSpacing(8);
    resultLayout->addWidget(m_flightList, 1);

    // 选中航班信息
    QLabel *detailLabel = new QLabel("航班详情");
    detailLabel->setStyleSheet("font-weight: bold; color: #555; margin-top: 12px;");
    resultLayout->addWidget(detailLabel);

    m_selectedFlightInfo = new QLabel("选择左侧航班查看详情");
    m_selectedFlightInfo->setStyleSheet(
        "background-color: #fff; "
        "padding: 16px; "
        "border-radius: 6px; "
        "border-left: 4px solid #ff9500; "
        "min-height: 100px; "
        "color: #666;"
    );
    m_selectedFlightInfo->setWordWrap(true);
    resultLayout->addWidget(m_selectedFlightInfo);

    // 预定按钮
    m_bookBtn = new QPushButton("✅ 确认预定");
    m_bookBtn->setCursor(Qt::PointingHandCursor);
    m_bookBtn->setEnabled(false);
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
