// dataanalyticswidget.cpp
#include "dataanalyticswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QListWidgetItem>
#include <QColor>
#include <QSize>

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
    QLabel *titleLabel = new QLabel("📊 数据分析");
    titleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #333; margin-bottom: 8px;");
    mainLayout->addWidget(titleLabel);

    // 标签页
    m_tabWidget = new QTabWidget();

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
    m_exportBtn = new QPushButton("💾 导出数据");
    m_exportBtn->setCursor(Qt::PointingHandCursor);
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
    )");
    m_totalFlightsCard->setMinimumHeight(80);

    m_activeFlightsCard = new QLabel();
    m_activeFlightsCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #ff9500;
    )");
    m_activeFlightsCard->setMinimumHeight(80);

    m_avgPriceCard = new QLabel();
    m_avgPriceCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #4caf50;
    )");
    m_avgPriceCard->setMinimumHeight(80);

    m_occupancyRateCard = new QLabel();
    m_occupancyRateCard->setStyleSheet(R"(
        background-color: #fff;
        border-radius: 8px;
        padding: 20px;
        border-left: 5px solid #f44336;
    )");
    m_occupancyRateCard->setMinimumHeight(80);
}

void DataAnalyticsWidget::setupRouteAnalysis()
{
    m_routeList = new QListWidget();
    m_routeList->setSpacing(8);
}

void DataAnalyticsWidget::setupPriceAnalysis()
{
    m_priceList = new QListWidget();
    m_priceList->setSpacing(8);
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
    // 统计数据 - 优化格式
    m_totalFlightsCard->setText(
        "<table style='width: 100%;'>"
        "<tr><td style='font-size: 16px;'><b>✈️ 总航班数</b></td><td style='text-align: right;'></td></tr>"
        "<tr><td><span style='font-size: 28px; color: #0078d4;'><b>156</b></span> 班</td></tr>"
        "</table>"
    );

    m_activeFlightsCard->setText(
        "<table style='width: 100%;'>"
        "<tr><td style='font-size: 16px;'><b>🚀 活跃航班</b></td><td style='text-align: right;'></td></tr>"
        "<tr><td><span style='font-size: 28px; color: #ff9500;'><b>89</b></span> 班</td></tr>"
        "</table>"
    );

    m_avgPriceCard->setText(
        "<table style='width: 100%;'>"
        "<tr><td style='font-size: 16px;'><b>💰 平均票价</b></td><td style='text-align: right;'></td></tr>"
        "<tr><td><span style='font-size: 28px; color: #4caf50;'><b>¥824</b></span></td></tr>"
        "</table>"
    );

    m_occupancyRateCard->setText(
        "<table style='width: 100%;'>"
        "<tr><td style='font-size: 16px;'><b>📊 座位占用率</b></td><td style='text-align: right;'></td></tr>"
        "<tr><td><span style='font-size: 28px; color: #f44336;'><b>82.5%</b></span></td></tr>"
        "</table>"
    );

    // 路线分析
    m_routeList->clear();
    QStringList routes = {
        "🛫 广州 → 上海：45班 | 平均¥824",
        "🛫 广州 → 北京：32班 | 平均¥945",
        "🛫 广州 → 深圳：28班 | 平均¥456",
        "🛫 广州 → 成都：18班 | 平均¥678",
        "🛫 广州 → 杭州：15班 | 平均¥567"
    };

    for (const QString& route : routes) {
        QListWidgetItem *item = new QListWidgetItem(route);
        item->setBackground(QColor(255, 255, 255));
        item->setSizeHint(QSize(0, 45));
        m_routeList->addItem(item);
    }

    // 价格分析
    m_priceList->clear();
    QStringList prices = {
        "💵 ¥0-500：23班 | 占比 14.7%",
        "💵 ¥500-1000：67班 | 占比 42.9%",
        "💵 ¥1000-2000：54班 | 占比 34.6%",
        "💵 ¥2000+：12班 | 占比 7.7%"
    };

    for (const QString& price : prices) {
        QListWidgetItem *item = new QListWidgetItem(price);
        item->setBackground(QColor(255, 255, 255));
        item->setSizeHint(QSize(0, 45));
        m_priceList->addItem(item);
    }

    // 详细报告
    m_reportText->setText(
        "<b>━━━━━━ 航班统计报告 ━━━━━━</b><br>"
        "<span style='color: #999;'>生成时间：2025-12-06 12:00:00</span><br><br>"
        "<b>【基本统计】</b><br>"
        "&nbsp;&nbsp;总航班数：<b>156班</b><br>"
        "&nbsp;&nbsp;活跃航班：<b>89班</b><br>"
        "&nbsp;&nbsp;已完成：<b>67班</b><br><br>"
        "<b>【座位统计】</b><br>"
        "&nbsp;&nbsp;总座位数：<b>45,300</b><br>"
        "&nbsp;&nbsp;已占座位：<b>37,387</b><br>"
        "&nbsp;&nbsp;可用座位：<b>7,913</b><br>"
        "&nbsp;&nbsp;座位占用率：<b>82.5%</b><br><br>"
        "<b>【票价统计】</b><br>"
        "&nbsp;&nbsp;最低价：<b>¥256</b><br>"
        "&nbsp;&nbsp;最高价：<b>¥2,450</b><br>"
        "&nbsp;&nbsp;平均价：<b>¥824</b><br><br>"
        "<b>━━━━━━ 报告结束 ━━━━━━</b>"
    );
}

void DataAnalyticsWidget::onRefreshData()
{
    loadStatistics();
}

void DataAnalyticsWidget::onExportData()
{
    // 导出数据
}

void DataAnalyticsWidget::onTabChanged(int index)
{
    Q_UNUSED(index);
}
