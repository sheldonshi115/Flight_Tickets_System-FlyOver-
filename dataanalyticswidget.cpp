// dataanalyticswidget.cpp
#include "dataanalyticswidget.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidgetItem>
#include <QColor>
#include <QSize>
#include <QDateTime>
#include <QMap>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>

DataAnalyticsWidget::DataAnalyticsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadStatistics();
}

DataAnalyticsWidget::~DataAnalyticsWidget()
{
}

void DataAnalyticsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // 标题
    QLabel *titleLabel = new QLabel("📊 航班数据分析");
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #333; margin-bottom: 8px; background: transparent;");
    mainLayout->addWidget(titleLabel);

    // 标签页
    m_tabWidget = new QTabWidget();
    m_tabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            background-color: #fff;
        }
        QTabBar::tab {
            background-color: #f5f5f5;
            color: #666;
            padding: 10px 20px;
            margin-right: 4px;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
        }
        QTabBar::tab:selected {
            background-color: #fff;
            color: #1976d2;
            font-weight: bold;
        }
        QTabBar::tab:hover {
            background-color: #e8e8e8;
        }
    )");

    // 基本统计页面
    QWidget *statsTab = new QWidget();
    QVBoxLayout *statsLayout = new QVBoxLayout(statsTab);
    statsLayout->setSpacing(12);
    setupStatsCards();
    statsLayout->addWidget(m_totalFlightsCard);
    statsLayout->addWidget(m_activeFlightsCard);
    statsLayout->addWidget(m_avgPriceCard);
    statsLayout->addWidget(m_occupancyRateCard);
    statsLayout->addStretch();
    m_tabWidget->addTab(statsTab, "📈 基本统计");

    // 路线分析页面
    QWidget *routeTab = new QWidget();
    QVBoxLayout *routeLayout = new QVBoxLayout(routeTab);
    setupRouteAnalysis();
    routeLayout->addWidget(m_routeList, 1);
    m_tabWidget->addTab(routeTab, "🗺️ 路线分析");

    // 价格分析页面
    QWidget *priceTab = new QWidget();
    QVBoxLayout *priceLayout = new QVBoxLayout(priceTab);
    setupPriceAnalysis();
    priceLayout->addWidget(m_priceList, 1);
    m_tabWidget->addTab(priceTab, "💵 价格分析");

    // 详细报告页面
    QWidget *reportTab = new QWidget();
    QVBoxLayout *reportLayout = new QVBoxLayout(reportTab);
    setupDetailedReport();
    reportLayout->addWidget(m_reportText, 1);
    m_tabWidget->addTab(reportTab, "📄 详细报告");

    mainLayout->addWidget(m_tabWidget, 1);

    // 底部按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton("🔄 刷新数据");
    m_refreshBtn->setCursor(Qt::PointingHandCursor);
    m_refreshBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1976d2;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1565c0;
        }
    )");
    
    m_exportBtn = new QPushButton("💾 导出报告");
    m_exportBtn->setCursor(Qt::PointingHandCursor);
    m_exportBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4caf50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #43a047;
        }
    )");
    
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addWidget(m_exportBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &DataAnalyticsWidget::onRefreshData);
    connect(m_exportBtn, &QPushButton::clicked, this, &DataAnalyticsWidget::onExportData);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &DataAnalyticsWidget::onTabChanged);
}

void DataAnalyticsWidget::setupStatsCards()
{
    m_totalFlightsCard = new QLabel();
    m_totalFlightsCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #0078d4;
        height: 60px;
    )");
    m_totalFlightsCard->setMinimumHeight(80);

    m_activeFlightsCard = new QLabel();
    m_activeFlightsCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #ff9500;
        height: 60px;
    )");
    m_activeFlightsCard->setMinimumHeight(80);

    m_avgPriceCard = new QLabel();
    m_avgPriceCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #4caf50;
        height: 60px;
    )");
    m_avgPriceCard->setMinimumHeight(80);

    m_occupancyRateCard = new QLabel();
    m_occupancyRateCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #f44336;
        height: 60px;
    )");
    m_occupancyRateCard->setMinimumHeight(80);
}

void DataAnalyticsWidget::setupRouteAnalysis()
{
    m_routeList = new QListWidget();
    m_routeList->setSpacing(8);
    m_routeList->setStyleSheet(R"(
        QListWidget {
            background-color: #fff;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
        }
        QListWidget::item {
            padding: 12px 16px;
            border-bottom: 1px solid #f0f0f0;
            font-size: 14px;
        }
        QListWidget::item:hover {
            background-color: #f5f5f5;
        }
    )");
}

void DataAnalyticsWidget::setupPriceAnalysis()
{
    m_priceList = new QListWidget();
    m_priceList->setSpacing(8);
    m_priceList->setStyleSheet(R"(
        QListWidget {
            background-color: #fff;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
        }
        QListWidget::item {
            padding: 12px 16px;
            border-bottom: 1px solid #f0f0f0;
            font-size: 14px;
        }
        QListWidget::item:hover {
            background-color: #f5f5f5;
        }
    )");
}

void DataAnalyticsWidget::setupDetailedReport()
{
    m_reportText = new QLabel();
    m_reportText->setStyleSheet(
        "background-color: #fff; "
        "padding: 16px; "
        "border-radius: 6px; "
        "color: #666;"
    );
    m_reportText->setWordWrap(true);
    m_reportText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
}

void DataAnalyticsWidget::loadStatistics()
{
    // 从数据库获取真实数据
    QList<Flight> allFlights = DBManager::instance().getAllFlights();
    QList<Order> allOrders = DBManager::instance().getAllOrders();
    
    int totalFlights = allFlights.size();
    int activeFlights = 0;
    int totalSeats = 0;
    int occupiedSeats = 0;
    double totalPrice = 0;
    double minPrice = 999999;
    double maxPrice = 0;
    
    // 路线统计 (出发城市->到达城市 -> 航班数和总价)
    QMap<QString, QPair<int, double>> routeStats;
    
    // 价格区间统计
    int priceRange0_500 = 0;
    int priceRange500_1000 = 0;
    int priceRange1000_2000 = 0;
    int priceRange2000Plus = 0;
    
    for (const Flight& flight : allFlights) {
        // 统计活跃航班（出发时间在当前时间之后）
        if (flight.departureTime() > QDateTime::currentDateTime()) {
            activeFlights++;
        }
        
        totalSeats += flight.totalSeats();
        occupiedSeats += (flight.totalSeats() - flight.availableSeats());
        totalPrice += flight.price();
        
        if (flight.price() < minPrice) minPrice = flight.price();
        if (flight.price() > maxPrice) maxPrice = flight.price();
        
        // 路线统计
        QString route = QString("%1 → %2").arg(flight.departureCity()).arg(flight.arrivalCity());
        if (routeStats.contains(route)) {
            routeStats[route].first++;
            routeStats[route].second += flight.price();
        } else {
            routeStats[route] = qMakePair(1, flight.price());
        }
        
        // 价格区间统计
        double price = flight.price();
        if (price < 500) priceRange0_500++;
        else if (price < 1000) priceRange500_1000++;
        else if (price < 2000) priceRange1000_2000++;
        else priceRange2000Plus++;
    }
    
    double avgPrice = totalFlights > 0 ? totalPrice / totalFlights : 0;
    double occupancyRate = totalSeats > 0 ? (double)occupiedSeats / totalSeats * 100 : 0;
    
    // 更新统计卡片
    m_totalFlightsCard->setText(
        QString("<table style='width: 100%;'>"
        "<tr><td style='font-size: 15px;'><b>✈️ 总航班数</b></td></tr>"
        "<tr><td><span style='font-size: 26px; height: 40px; color: #0078d4;'><b>%1</b></span> 班</td></tr>"
        "</table>").arg(totalFlights)
    );

    m_activeFlightsCard->setText(
        QString("<table style='width: 100%;'>"
        "<tr><td style='font-size: 15px;'><b>🚀 活跃航班</b></td></tr>"
        "<tr><td><span style='font-size: 26px; height: 40px; color: #ff9500;'><b>%1</b></span> 班</td></tr>"
        "</table>").arg(activeFlights)
    );

    m_avgPriceCard->setText(
        QString("<table style='width: 100%;'>"
        "<tr><td style='font-size: 15px;'><b>💰 平均票价</b></td></tr>"
        "<tr><td><span style='font-size: 26px; height: 40px; color: #4caf50;'><b>¥%1</b></span></td></tr>"
        "</table>").arg(QString::number(avgPrice, 'f', 0))
    );

    m_occupancyRateCard->setText(
        QString("<table style='width: 100%;'>"
        "<tr><td style='font-size: 15px;'><b>📊 座位占用率</b></td></tr>"
        "<tr><td><span style='font-size: 26px; height: 40px; color: #f44336;'><b>%1%</b></span></td></tr>"
        "</table>").arg(QString::number(occupancyRate, 'f', 1))
    );

    // 路线分析 - 按航班数排序
    m_routeList->clear();
    QList<QPair<QString, QPair<int, double>>> sortedRoutes;
    for (auto it = routeStats.begin(); it != routeStats.end(); ++it) {
        sortedRoutes.append(qMakePair(it.key(), it.value()));
    }
    std::sort(sortedRoutes.begin(), sortedRoutes.end(), 
        [](const auto& a, const auto& b) { return a.second.first > b.second.first; });
    
    for (const auto& route : sortedRoutes) {
        double routeAvgPrice = route.second.first > 0 ? route.second.second / route.second.first : 0;
        QString text = QString("🛫 %1：%2班 | 平均¥%3")
            .arg(route.first)
            .arg(route.second.first)
            .arg(QString::number(routeAvgPrice, 'f', 0));
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setBackground(QColor(255, 255, 255));
        item->setSizeHint(QSize(0, 50));
        m_routeList->addItem(item);
    }

    // 价格分析
    m_priceList->clear();
    auto addPriceItem = [this, totalFlights](const QString& range, int count) {
        double percent = totalFlights > 0 ? (double)count / totalFlights * 100 : 0;
        QString text = QString("💵 %1：%2班 | 占比 %3%")
            .arg(range).arg(count).arg(QString::number(percent, 'f', 1));
        QListWidgetItem *item = new QListWidgetItem(text);
        item->setBackground(QColor(255, 255, 255));
        item->setSizeHint(QSize(0, 50));
        m_priceList->addItem(item);
    };
    
    addPriceItem("¥0-500", priceRange0_500);
    addPriceItem("¥500-1000", priceRange500_1000);
    addPriceItem("¥1000-2000", priceRange1000_2000);
    addPriceItem("¥2000+", priceRange2000Plus);

    // 详细报告
    QString reportTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    m_reportText->setText(
        QString("<b>━━━━━━ 航班统计报告 ━━━━━━</b><br>"
        "<span style='color: #999;'>生成时间：%1</span><br><br>"
        "<b>【基本统计】</b><br>"
        "&nbsp;&nbsp;总航班数：<b>%2班</b><br>"
        "&nbsp;&nbsp;活跃航班：<b>%3班</b><br>"
        "&nbsp;&nbsp;已完成：<b>%4班</b><br><br>"
        "<b>【座位统计】</b><br>"
        "&nbsp;&nbsp;总座位数：<b>%5</b><br>"
        "&nbsp;&nbsp;已占座位：<b>%6</b><br>"
        "&nbsp;&nbsp;可用座位：<b>%7</b><br>"
        "&nbsp;&nbsp;座位占用率：<b>%8%</b><br><br>"
        "<b>【票价统计】</b><br>"
        "&nbsp;&nbsp;最低价：<b>¥%9</b><br>"
        "&nbsp;&nbsp;最高价：<b>¥%10</b><br>"
        "&nbsp;&nbsp;平均价：<b>¥%11</b><br><br>"
        "<b>【订单统计】</b><br>"
        "&nbsp;&nbsp;总订单数：<b>%12</b><br><br>"
        "<b>━━━━━━ 报告结束 ━━━━━━</b>")
        .arg(reportTime)
        .arg(totalFlights)
        .arg(activeFlights)
        .arg(totalFlights - activeFlights)
        .arg(totalSeats)
        .arg(occupiedSeats)
        .arg(totalSeats - occupiedSeats)
        .arg(QString::number(occupancyRate, 'f', 1))
        .arg(totalFlights > 0 ? QString::number(minPrice, 'f', 0) : "0")
        .arg(QString::number(maxPrice, 'f', 0))
        .arg(QString::number(avgPrice, 'f', 0))
        .arg(allOrders.size())
    );
}

void DataAnalyticsWidget::onRefreshData()
{
    loadStatistics();
    QMessageBox::information(this, "刷新成功", "数据已更新！");
}

void DataAnalyticsWidget::onExportData()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出报告", 
        QString("航班分析报告_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        "文本文件 (*.txt)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建文件！");
        return;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // 获取数据
    QList<Flight> allFlights = DBManager::instance().getAllFlights();
    QList<Order> allOrders = DBManager::instance().getAllOrders();
    
    int totalFlights = allFlights.size();
    int activeFlights = 0;
    int totalSeats = 0;
    int occupiedSeats = 0;
    double totalPrice = 0;
    
    for (const Flight& flight : allFlights) {
        if (flight.departureTime() > QDateTime::currentDateTime()) {
            activeFlights++;
        }
        totalSeats += flight.totalSeats();
        occupiedSeats += (flight.totalSeats() - flight.availableSeats());
        totalPrice += flight.price();
    }
    
    double avgPrice = totalFlights > 0 ? totalPrice / totalFlights : 0;
    double occupancyRate = totalSeats > 0 ? (double)occupiedSeats / totalSeats * 100 : 0;
    
    out << "============================================\n";
    out << "        航班票务系统 - 数据分析报告\n";
    out << "============================================\n\n";
    out << "生成时间：" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";
    
    out << "【基本统计】\n";
    out << "  总航班数：" << totalFlights << " 班\n";
    out << "  活跃航班：" << activeFlights << " 班\n";
    out << "  已完成航班：" << (totalFlights - activeFlights) << " 班\n\n";
    
    out << "【座位统计】\n";
    out << "  总座位数：" << totalSeats << "\n";
    out << "  已占座位：" << occupiedSeats << "\n";
    out << "  可用座位：" << (totalSeats - occupiedSeats) << "\n";
    out << "  座位占用率：" << QString::number(occupancyRate, 'f', 1) << "%\n\n";
    
    out << "【票价统计】\n";
    out << "  平均票价：¥" << QString::number(avgPrice, 'f', 0) << "\n\n";
    
    out << "【订单统计】\n";
    out << "  总订单数：" << allOrders.size() << "\n\n";
    
    out << "============================================\n";
    out << "                 报告结束\n";
    out << "============================================\n";
    
    file.close();
    QMessageBox::information(this, "导出成功", QString("报告已保存到：\n%1").arg(fileName));
}

void DataAnalyticsWidget::onTabChanged(int index)
{
    Q_UNUSED(index);
}
