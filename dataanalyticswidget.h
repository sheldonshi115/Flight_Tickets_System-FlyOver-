// dataanalyticswidget.h
#ifndef DATAANALYTICSWIDGET_H
#define DATAANALYTICSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>

class DataAnalyticsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataAnalyticsWidget(QWidget *parent = nullptr);
    ~DataAnalyticsWidget();

private slots:
    void onRefreshData();
    void onExportData();
    void onTabChanged(int index);
    void loadStatistics();

private:
    void setupUI();
    void setupStatsCards();
    void setupRouteAnalysis();
    void setupPriceAnalysis();
    void setupDetailedReport();

    // 统计卡片
    QLabel *m_totalFlightsCard;
    QLabel *m_activeFlightsCard;
    QLabel *m_avgPriceCard;
    QLabel *m_occupancyRateCard;

    // 路线分析
    QListWidget *m_routeList;

    // 价格分析
    QListWidget *m_priceList;

    // 详细报告
    QLabel *m_reportText;

    QTabWidget *m_tabWidget;
    QPushButton *m_refreshBtn;
    QPushButton *m_exportBtn;
};

#endif // DATAANALYTICSWIDGET_H
