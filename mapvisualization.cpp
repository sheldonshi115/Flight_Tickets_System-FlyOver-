#include "mapvisualization.h"
#include <QPainterPath>
#include <QDebug>
#include <cmath>

MapVisualization::MapVisualization(QWidget *parent)
    : QWidget(parent)
    , m_minLat(18.0)
    , m_maxLat(53.0)
    , m_minLon(73.0)
    , m_maxLon(135.0)
    , m_margin(50)
{
    setMouseTracking(true);
    setMinimumSize(800, 600);
    
    initChinaCities();
}

MapVisualization::~MapVisualization()
{
}

void MapVisualization::initChinaCities()
{
    // 添加中国所有主要城市坐标（经纬度）- 完整版
    // 直辖市
    addCity("北京", 39.9042, 116.4074);
    addCity("上海", 31.2304, 121.4737);
    addCity("天津", 39.3434, 117.3616);
    addCity("重庆", 29.4316, 106.9123);
    
    // 省会城市
    addCity("广州", 23.1291, 113.2644);
    addCity("成都", 30.5728, 104.0668);
    addCity("杭州", 30.2741, 120.1551);
    addCity("西安", 34.3416, 108.9398);
    addCity("武汉", 30.5928, 114.3055);
    addCity("郑州", 34.7466, 113.6253);
    addCity("南京", 32.0603, 118.7969);
    addCity("长沙", 28.2282, 112.9388);
    addCity("沈阳", 41.8057, 123.4328);
    addCity("昆明", 25.0406, 102.7123);
    addCity("哈尔滨", 45.8038, 126.5340);
    addCity("乌鲁木齐", 43.8256, 87.6168);
    addCity("拉萨", 29.6500, 91.1000);
    addCity("海口", 20.0444, 110.1999);
    addCity("南宁", 22.8170, 108.3665);
    addCity("福州", 26.0745, 119.2965);
    addCity("贵阳", 26.6470, 106.6302);
    addCity("兰州", 36.0611, 103.8343);
    addCity("银川", 38.4872, 106.2309);
    addCity("西宁", 36.6171, 101.7782);
    addCity("呼和浩特", 40.8414, 111.7519);
    addCity("太原", 37.8706, 112.5489);
    addCity("石家庄", 38.0428, 114.5149);
    addCity("济南", 36.6512, 117.1205);
    addCity("南昌", 28.6829, 115.8579);
    addCity("合肥", 31.8206, 117.2272);
    addCity("长春", 43.8171, 125.3235);
    
    // 重要地级市
    addCity("深圳", 22.5431, 114.0579);
    addCity("青岛", 36.0671, 120.3826);
    addCity("大连", 38.9140, 121.6147);
    addCity("厦门", 24.4798, 118.0894);
    addCity("宁波", 29.8683, 121.5440);
    addCity("苏州", 31.2989, 120.5853);
    addCity("无锡", 31.4912, 120.3119);
    addCity("常州", 31.8111, 119.9741);
    addCity("南通", 32.0085, 120.8947);
    addCity("徐州", 34.2044, 117.2845);
    addCity("扬州", 32.3912, 119.4215);
    addCity("温州", 28.0006, 120.6989);
    addCity("嘉兴", 30.7467, 120.7505);
    addCity("金华", 29.0788, 119.6478);
    addCity("台州", 28.6568, 121.4206);
    addCity("绍兴", 30.0333, 120.5800);
    addCity("三亚", 18.2528, 109.5117);
    addCity("桂林", 25.2736, 110.2901);
    addCity("泉州", 24.8741, 118.6757);
    addCity("珠海", 22.2710, 113.5767);
    addCity("惠州", 23.1115, 114.4152);
    addCity("东莞", 23.0205, 113.7518);
    addCity("佛山", 23.0218, 113.1219);
    addCity("中山", 22.5171, 113.3926);
    addCity("江门", 22.5789, 113.0819);
    addCity("湛江", 21.2707, 110.3594);
    addCity("汕头", 23.3540, 116.6818);
    addCity("烟台", 37.4638, 121.4478);
    addCity("威海", 37.5128, 122.1201);
    addCity("潍坊", 36.7069, 119.1619);
    addCity("临沂", 35.1041, 118.3563);
    addCity("洛阳", 34.6196, 112.4539);
    addCity("开封", 34.7971, 114.3074);
    addCity("南阳", 32.9907, 112.5285);
    addCity("保定", 38.8738, 115.4649);
    addCity("唐山", 39.6304, 118.1802);
    addCity("秦皇岛", 39.9350, 119.6000);
    addCity("包头", 40.6575, 109.8401);
    addCity("鞍山", 41.1106, 122.9945);
    addCity("抚顺", 41.8579, 123.9571);
    addCity("本溪", 41.2979, 123.7654);
    addCity("丹东", 40.1244, 124.3831);
    addCity("吉林", 43.8376, 126.5494);
    addCity("齐齐哈尔", 47.3543, 123.9182);
    addCity("大庆", 46.5896, 125.1036);
    addCity("牡丹江", 44.5519, 129.6338);
    addCity("绵阳", 31.4678, 104.6793);
    addCity("德阳", 31.1270, 104.3979);
    addCity("南充", 30.8378, 106.1105);
    addCity("宜宾", 28.7516, 104.6430);
    addCity("遵义", 27.7256, 106.9272);
    addCity("大理", 25.6065, 100.2671);
    addCity("丽江", 26.8559, 100.2271);
    addCity("西双版纳", 22.0089, 100.7977);
    
    calculateScreenPositions();
}

void MapVisualization::addCity(const QString& name, double lat, double lon)
{
    CityCoordinate coord;
    coord.cityName = name;
    coord.latitude = lat;
    coord.longitude = lon;
    m_cities[name] = coord;
}

void MapVisualization::setFlightData(const QList<Flight>& flights)
{
    m_flights = flights;
    m_routes.clear();
    
    // 统计航线数据
    QMap<QString, FlightRoute> routeMap;
    
    for (const Flight& flight : flights) {
        QString routeKey = flight.departureCity() + "-" + flight.arrivalCity();
        
        if (!routeMap.contains(routeKey)) {
            FlightRoute route;
            route.departure = flight.departureCity();
            route.destination = flight.arrivalCity();
            route.flightCount = 1;
            route.avgPrice = flight.price();
            routeMap[routeKey] = route;
        } else {
            FlightRoute& route = routeMap[routeKey];
            route.flightCount++;
            route.avgPrice = (route.avgPrice * (route.flightCount - 1) + flight.price()) / route.flightCount;
        }
    }
    
    // 标记热门航线（航班数量前20%）
    m_routes = routeMap.values();
    std::sort(m_routes.begin(), m_routes.end(), 
              [](const FlightRoute& a, const FlightRoute& b) {
                  return a.flightCount > b.flightCount;
              });
    
    int popularCount = qMax(1, m_routes.size() / 5);
    for (int i = 0; i < qMin(popularCount, m_routes.size()); ++i) {
        m_routes[i].isPopular = true;
    }
    
    update();
}

void MapVisualization::clearData()
{
    m_flights.clear();
    m_routes.clear();
    update();
}

void MapVisualization::highlightRoute(const QString& departure, const QString& destination)
{
    m_highlightedRoute = departure + "-" + destination;
    update();
}

void MapVisualization::calculateScreenPositions()
{
    for (auto it = m_cities.begin(); it != m_cities.end(); ++it) {
        it->screenPos = latLonToScreen(it->latitude, it->longitude);
    }
}

QPointF MapVisualization::latLonToScreen(double lat, double lon)
{
    double x = (lon - m_minLon) / (m_maxLon - m_minLon) * (width() - 2 * m_margin) + m_margin;
    double y = (m_maxLat - lat) / (m_maxLat - m_minLat) * (height() - 2 * m_margin) + m_margin;
    return QPointF(x, y);
}

void MapVisualization::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 背景渐变
    QLinearGradient bgGradient(0, 0, 0, height());
    bgGradient.setColorAt(0, QColor(240, 249, 255));
    bgGradient.setColorAt(1, QColor(224, 242, 254));
    painter.fillRect(rect(), bgGradient);
    
    drawMap(painter);
    drawRoutes(painter);
    drawCities(painter);
    drawLegend(painter);
    
    if (!m_hoveredCity.isEmpty()) {
        drawTooltip(painter);
    }
}

void MapVisualization::drawMap(QPainter& painter)
{
    // 绘制中国地图轮廓（简化版）
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.setBrush(QColor(255, 255, 255, 200));
    
    // 这里简化绘制一个矩形框代表地图区域
    QRect mapRect(m_margin, m_margin, 
                  width() - 2 * m_margin, 
                  height() - 2 * m_margin);
    painter.drawRect(mapRect);
    
    // 绘制网格
    painter.setPen(QPen(QColor(230, 230, 230), 1, Qt::DotLine));
    for (int i = 1; i < 10; ++i) {
        int x = m_margin + (width() - 2 * m_margin) * i / 10;
        painter.drawLine(x, m_margin, x, height() - m_margin);
        
        int y = m_margin + (height() - 2 * m_margin) * i / 10;
        painter.drawLine(m_margin, y, width() - m_margin, y);
    }
}

void MapVisualization::drawCities(QPainter& painter)
{
    for (auto it = m_cities.begin(); it != m_cities.end(); ++it) {
        QPointF pos = it->screenPos;
        
        bool isHovered = (it.key() == m_hoveredCity);
        
        // 绘制城市点
        if (isHovered) {
            painter.setPen(QPen(QColor(59, 130, 246), 3));
            painter.setBrush(QColor(96, 165, 250));
        } else {
            painter.setPen(QPen(QColor(100, 100, 100), 2));
            painter.setBrush(QColor(59, 130, 246));
        }
        
        double radius = isHovered ? 8 : 6;
        painter.drawEllipse(pos, radius, radius);
        
        // 绘制城市名称
        painter.setPen(QColor(71, 85, 105));
        QFont font = painter.font();
        font.setPointSize(isHovered ? 10 : 9);
        font.setBold(isHovered);
        painter.setFont(font);
        
        painter.drawText(QRectF(pos.x() - 40, pos.y() + 10, 80, 20),
                        Qt::AlignCenter, it->cityName);
    }
}

void MapVisualization::drawRoutes(QPainter& painter)
{
    for (const FlightRoute& route : m_routes) {
        if (!m_cities.contains(route.departure) || 
            !m_cities.contains(route.destination)) {
            continue;
        }
        
        QPointF start = m_cities[route.departure].screenPos;
        QPointF end = m_cities[route.destination].screenPos;
        
        QString routeKey = route.departure + "-" + route.destination;
        bool isHighlighted = (routeKey == m_highlightedRoute);
        
        // 绘制航线
        QPen pen;
        if (isHighlighted) {
            pen = QPen(QColor(239, 68, 68), 3);
        } else if (route.isPopular) {
            pen = QPen(QColor(59, 130, 246, 150), 2);
        } else {
            pen = QPen(QColor(148, 163, 184, 80), 1);
        }
        
        painter.setPen(pen);
        
        // 绘制曲线（贝塞尔曲线）
        QPainterPath path;
        path.moveTo(start);
        
        QPointF ctrl = QPointF((start.x() + end.x()) / 2,
                               qMin(start.y(), end.y()) - 50);
        path.quadTo(ctrl, end);
        
        painter.drawPath(path);
        
        // 绘制箭头
        if (isHighlighted || route.isPopular) {
            double angle = std::atan2(end.y() - ctrl.y(), end.x() - ctrl.x());
            QPointF arrowP1 = end - QPointF(std::cos(angle + M_PI / 6) * 10,
                                            std::sin(angle + M_PI / 6) * 10);
            QPointF arrowP2 = end - QPointF(std::cos(angle - M_PI / 6) * 10,
                                            std::sin(angle - M_PI / 6) * 10);
            
            painter.setBrush(pen.color());
            QPolygonF arrow;
            arrow << end << arrowP1 << arrowP2;
            painter.drawPolygon(arrow);
        }
    }
}

void MapVisualization::drawLegend(QPainter& painter)
{
    int legendX = width() - 180;
    int legendY = 20;
    
    // 图例背景
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.setPen(QPen(QColor(226, 232, 240), 2));
    painter.drawRoundedRect(legendX, legendY, 160, 120, 10, 10);
    
    painter.setPen(QColor(71, 85, 105));
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(legendX + 10, legendY + 20, "图例");
    
    font.setBold(false);
    font.setPointSize(8);
    painter.setFont(font);
    
    // 热门航线
    painter.setPen(QPen(QColor(59, 130, 246), 2));
    painter.drawLine(legendX + 10, legendY + 40, legendX + 40, legendY + 40);
    painter.setPen(QColor(71, 85, 105));
    painter.drawText(legendX + 50, legendY + 45, "热门航线");
    
    // 普通航线
    painter.setPen(QPen(QColor(148, 163, 184), 1));
    painter.drawLine(legendX + 10, legendY + 60, legendX + 40, legendY + 60);
    painter.setPen(QColor(71, 85, 105));
    painter.drawText(legendX + 50, legendY + 65, "普通航线");
    
    // 城市标记
    painter.setBrush(QColor(59, 130, 246));
    painter.drawEllipse(QPointF(legendX + 25, legendY + 80), 5, 5);
    painter.setPen(QColor(71, 85, 105));
    painter.drawText(legendX + 50, legendY + 85, "城市");
    
    // 统计信息
    painter.drawText(legendX + 10, legendY + 105, 
                    QString("航线总数: %1").arg(m_routes.size()));
}

void MapVisualization::drawTooltip(QPainter& painter)
{
    if (m_hoveredCity.isEmpty()) return;
    
    // 统计从该城市出发的航班
    int departCount = 0;
    int arriveCount = 0;
    
    for (const Flight& flight : m_flights) {
        if (flight.departureCity() == m_hoveredCity) departCount++;
        if (flight.arrivalCity() == m_hoveredCity) arriveCount++;
    }
    
    QString tooltip = QString("%1\n出发: %2 班\n到达: %3 班")
                        .arg(m_hoveredCity)
                        .arg(departCount)
                        .arg(arriveCount);
    
    // 绘制工具提示
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(QRect(), Qt::AlignLeft, tooltip);
    textRect.adjust(-10, -5, 10, 5);
    textRect.moveTopLeft(m_mousePos.toPoint() + QPoint(15, 15));
    
    painter.setBrush(QColor(30, 41, 59, 230));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 5, 5);
    
    painter.setPen(Qt::white);
    painter.drawText(textRect, Qt::AlignCenter, tooltip);
}

void MapVisualization::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->position();
    QString oldHovered = m_hoveredCity;
    m_hoveredCity = getCityAtPoint(m_mousePos);
    
    if (oldHovered != m_hoveredCity) {
        update();
    }
}

void MapVisualization::mousePressEvent(QMouseEvent *event)
{
    QString city = getCityAtPoint(event->position());
    if (!city.isEmpty()) {
        emit cityClicked(city);
    }
}

void MapVisualization::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    calculateScreenPositions();
}

QString MapVisualization::getCityAtPoint(const QPointF& point)
{
    for (auto it = m_cities.begin(); it != m_cities.end(); ++it) {
        QPointF cityPos = it->screenPos;
        double distance = std::sqrt(std::pow(point.x() - cityPos.x(), 2) +
                                   std::pow(point.y() - cityPos.y(), 2));
        
        if (distance < 15) { // 15像素范围内
            return it.key();
        }
    }
    
    return QString();
}
