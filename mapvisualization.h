#ifndef MAPVISUALIZATION_H
#define MAPVISUALIZATION_H

#include <QWidget>
#include <QMap>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPolygonF>
#include "flight.h"

// 城市坐标结构
struct CityCoordinate {
    QString cityName;
    double latitude;   // 纬度
    double longitude;  // 经度
    QPointF screenPos; // 屏幕坐标
};

// 航线信息
struct FlightRoute {
    QString departure;
    QString destination;
    int flightCount = 0;    // 该航线的航班数量
    double avgPrice = 0.0;  // 平均价格
    bool isPopular = false; // 是否热门航线
};

// 地图可视化组件 - 支持拖拽和缩放
class MapVisualization : public QWidget
{
    Q_OBJECT

public:
    explicit MapVisualization(QWidget *parent = nullptr);
    ~MapVisualization();

    // 设置航班数据
    void setFlightData(const QList<Flight>& flights);
    
    // 添加城市
    void addCity(const QString& name, double lat, double lon);
    
    // 清空数据
    void clearData();
    
    // 高亮某条航线
    void highlightRoute(const QString& departure, const QString& destination);
    
    // 重置视图
    void resetView();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void cityClicked(const QString& cityName);
    void routeClicked(const QString& departure, const QString& destination);
    void zoomChanged(double zoomLevel);

private:
    void initChinaCities();
    void initChinaMapOutline();
    void calculateScreenPositions();
    void drawMap(QPainter& painter);
    void drawChinaOutline(QPainter& painter);
    void drawCities(QPainter& painter);
    void drawRoutes(QPainter& painter);
    void drawLegend(QPainter& painter);
    void drawZoomIndicator(QPainter& painter);
    void drawTooltip(QPainter& painter);
    
    QPointF latLonToScreen(double lat, double lon);
    QPointF screenToLatLon(const QPointF& screenPos);
    QString getCityAtPoint(const QPointF& point);
    
    QMap<QString, CityCoordinate> m_cities;
    QList<FlightRoute> m_routes;
    QList<Flight> m_flights;
    
    QString m_highlightedRoute;
    QString m_hoveredCity;
    QPointF m_mousePos;
    
    // 地图投影参数
    double m_minLat, m_maxLat;
    double m_minLon, m_maxLon;
    int m_margin;
    
    // 拖拽和缩放参数
    double m_zoomLevel;          // 缩放级别 (1.0 = 100%)
    double m_minZoom;            // 最小缩放
    double m_maxZoom;            // 最大缩放
    QPointF m_panOffset;         // 平移偏移
    bool m_isDragging;           // 是否正在拖拽
    QPointF m_lastMousePos;      // 上次鼠标位置
    
    // 中国地图轮廓
    QList<QPolygonF> m_chinaOutline;
};

#endif // MAPVISUALIZATION_H
