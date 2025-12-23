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
    , m_zoomLevel(1.0)
    , m_minZoom(0.5)
    , m_maxZoom(3.0)
    , m_panOffset(0, 0)
    , m_isDragging(false)
{
    setMouseTracking(true);
    setMinimumSize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    
    initChinaCities();
    initChinaMapOutline();
}

MapVisualization::~MapVisualization()
{
}

void MapVisualization::initChinaMapOutline()
{
    // 简化的中国地图轮廓（主要边界点）
    QPolygonF mainOutline;
    
    // 中国大陆主要边界点（经纬度简化）
    mainOutline << QPointF(121.5, 53.3)   // 东北-漠河
                << QPointF(135.0, 48.4)   // 东北角
                << QPointF(131.0, 43.0)   // 珲春
                << QPointF(129.5, 42.0)
                << QPointF(124.4, 40.0)   // 丹东
                << QPointF(122.1, 37.5)   // 烟台
                << QPointF(121.5, 31.2)   // 上海
                << QPointF(120.3, 27.0)   // 温州
                << QPointF(118.1, 24.5)   // 厦门
                << QPointF(117.0, 23.5)   // 汕头
                << QPointF(114.1, 22.5)   // 深圳
                << QPointF(113.5, 22.2)   // 香港
                << QPointF(110.3, 20.0)   // 海南北
                << QPointF(108.6, 18.2)   // 三亚
                << QPointF(109.1, 21.5)   // 北海
                << QPointF(106.6, 22.8)   // 南宁
                << QPointF(103.8, 22.0)   // 云南南
                << QPointF(97.5, 21.5)    // 西双版纳
                << QPointF(97.0, 28.0)    // 云南西
                << QPointF(92.0, 28.0)    // 西藏东南
                << QPointF(79.0, 32.0)    // 西藏西南
                << QPointF(74.0, 37.0)    // 新疆西南
                << QPointF(73.5, 39.5)    // 喀什
                << QPointF(79.9, 45.0)    // 新疆北
                << QPointF(87.6, 49.2)    // 阿勒泰
                << QPointF(97.0, 49.0)    // 内蒙古西
                << QPointF(111.5, 43.8)   // 内蒙古中
                << QPointF(119.9, 49.2)   // 满洲里
                << QPointF(121.5, 53.3);  // 闭合到漠河
    
    m_chinaOutline.append(mainOutline);
    
    // 海南岛
    QPolygonF hainan;
    hainan << QPointF(110.3, 20.0)
           << QPointF(111.0, 19.2)
           << QPointF(110.5, 18.2)
           << QPointF(108.6, 18.2)
           << QPointF(108.6, 19.5)
           << QPointF(110.3, 20.0);
    m_chinaOutline.append(hainan);
    
    // 台湾
    QPolygonF taiwan;
    taiwan << QPointF(121.5, 25.3)
           << QPointF(122.0, 24.5)
           << QPointF(121.5, 22.0)
           << QPointF(120.2, 22.5)
           << QPointF(120.0, 24.5)
           << QPointF(121.5, 25.3);
    m_chinaOutline.append(taiwan);
}

void MapVisualization::initChinaCities()
{
    // 添加中国所有主要城市坐标（经纬度）
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
    addCity("三亚", 18.2528, 109.5117);
    addCity("桂林", 25.2736, 110.2901);
    addCity("珠海", 22.2710, 113.5767);
    addCity("东莞", 23.0205, 113.7518);
    addCity("佛山", 23.0218, 113.1219);
    addCity("烟台", 37.4638, 121.4478);
    addCity("威海", 37.5128, 122.1201);
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

void MapVisualization::resetView()
{
    m_zoomLevel = 1.0;
    m_panOffset = QPointF(0, 0);
    calculateScreenPositions();
    emit zoomChanged(m_zoomLevel);
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
    // 基础转换
    double baseX = (lon - m_minLon) / (m_maxLon - m_minLon) * (width() - 2 * m_margin) + m_margin;
    double baseY = (m_maxLat - lat) / (m_maxLat - m_minLat) * (height() - 2 * m_margin) + m_margin;
    
    // 应用缩放和平移
    QPointF center(width() / 2.0, height() / 2.0);
    double scaledX = (baseX - center.x()) * m_zoomLevel + center.x() + m_panOffset.x();
    double scaledY = (baseY - center.y()) * m_zoomLevel + center.y() + m_panOffset.y();
    
    return QPointF(scaledX, scaledY);
}

QPointF MapVisualization::screenToLatLon(const QPointF& screenPos)
{
    QPointF center(width() / 2.0, height() / 2.0);
    
    // 逆变换
    double baseX = (screenPos.x() - m_panOffset.x() - center.x()) / m_zoomLevel + center.x();
    double baseY = (screenPos.y() - m_panOffset.y() - center.y()) / m_zoomLevel + center.y();
    
    double lon = (baseX - m_margin) / (width() - 2 * m_margin) * (m_maxLon - m_minLon) + m_minLon;
    double lat = m_maxLat - (baseY - m_margin) / (height() - 2 * m_margin) * (m_maxLat - m_minLat);
    
    return QPointF(lon, lat);
}

void MapVisualization::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 背景渐变
    QLinearGradient bgGradient(0, 0, 0, height());
    bgGradient.setColorAt(0, QColor(235, 248, 255));
    bgGradient.setColorAt(0.5, QColor(219, 241, 255));
    bgGradient.setColorAt(1, QColor(207, 237, 255));
    painter.fillRect(rect(), bgGradient);
    
    // 重新计算城市位置
    calculateScreenPositions();
    
    drawMap(painter);
    drawChinaOutline(painter);
    drawRoutes(painter);
    drawCities(painter);
    drawLegend(painter);
    drawZoomIndicator(painter);
    
    if (!m_hoveredCity.isEmpty()) {
        drawTooltip(painter);
    }
}

void MapVisualization::drawMap(QPainter& painter)
{
    // 绘制地图区域边框
    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.setBrush(Qt::NoBrush);
    
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

void MapVisualization::drawChinaOutline(QPainter& painter)
{
    // 绘制中国地图轮廓
    painter.setPen(QPen(QColor(59, 130, 246, 150), 2));
    painter.setBrush(QColor(59, 130, 246, 30));
    
    for (const QPolygonF& outline : m_chinaOutline) {
        QPolygonF screenPolygon;
        for (const QPointF& point : outline) {
            // point.x() = longitude, point.y() = latitude
            QPointF screenPoint = latLonToScreen(point.y(), point.x());
            screenPolygon << screenPoint;
        }
        painter.drawPolygon(screenPolygon);
    }
}

void MapVisualization::drawCities(QPainter& painter)
{
    for (auto it = m_cities.begin(); it != m_cities.end(); ++it) {
        QPointF pos = it->screenPos;
        
        // 检查是否在可视区域内
        if (pos.x() < 0 || pos.x() > width() || pos.y() < 0 || pos.y() > height()) {
            continue;
        }
        
        bool isHovered = (it.key() == m_hoveredCity);
        
        // 根据缩放级别调整城市点大小
        double baseRadius = 7 * m_zoomLevel;
        double radius = isHovered ? baseRadius * 1.3 : baseRadius;
        radius = qBound(4.0, radius, 15.0);
        
        // 绘制城市点
        if (isHovered) {
            // 悬停时的光晕效果
            painter.setPen(QPen(QColor(59, 130, 246, 100), 6));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(pos, radius + 4, radius + 4);
            
            painter.setPen(QPen(QColor(37, 99, 235), 3));
            painter.setBrush(QColor(96, 165, 250));
        } else {
            painter.setPen(QPen(QColor(37, 99, 235), 2.5));
            painter.setBrush(QColor(59, 130, 246));
        }
        
        painter.drawEllipse(pos, radius, radius);
        
        // 只在足够缩放时显示城市名称
        if (m_zoomLevel >= 0.8) {
            painter.setPen(QColor(30, 58, 138));
            QFont font = painter.font();
            int fontSize = qBound(8, (int)(9 * m_zoomLevel), 14);
            font.setPointSize(isHovered ? fontSize + 2 : fontSize);
            font.setBold(isHovered);
            font.setFamily("Microsoft YaHei UI");
            painter.setFont(font);
            
            painter.drawText(QRectF(pos.x() - 40, pos.y() + radius + 4, 80, 20),
                            Qt::AlignCenter, it->cityName);
        }
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
        
        // 检查是否在可视区域内
        QRectF viewRect(0, 0, width(), height());
        if (!viewRect.contains(start) && !viewRect.contains(end)) {
            continue;
        }
        
        QString routeKey = route.departure + "-" + route.destination;
        bool isHighlighted = (routeKey == m_highlightedRoute);
        
        // 设置航线样式
        QPen pen;
        if (isHighlighted) {
            pen = QPen(QColor(239, 68, 68), 3.5 * m_zoomLevel);
        } else if (route.isPopular) {
            pen = QPen(QColor(59, 130, 246, 180), 2.5 * m_zoomLevel);
        } else {
            pen = QPen(QColor(148, 163, 184, 120), 1.5 * m_zoomLevel);
        }
        
        painter.setPen(pen);
        
        // 绘制直线连接
        painter.drawLine(start, end);
        
        // 在航线中点显示航班数量
        if (route.flightCount > 0 && m_zoomLevel >= 0.8) {
            QPointF mid = (start + end) / 2;
            
            // 绘制航班数量标签背景
            QString countText = QString::number(route.flightCount);
            QFont font = painter.font();
            font.setPointSize(9);
            font.setBold(true);
            painter.setFont(font);
            
            QFontMetrics fm(font);
            int textWidth = fm.horizontalAdvance(countText) + 10;
            int textHeight = fm.height() + 4;
            
            QRectF labelRect(mid.x() - textWidth/2, mid.y() - textHeight/2, textWidth, textHeight);
            
            // 背景
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(255, 255, 255, 220));
            painter.drawRoundedRect(labelRect, 4, 4);
            
            // 文字
            painter.setPen(QColor(59, 130, 246));
            painter.drawText(labelRect, Qt::AlignCenter, countText);
        }
        
        // 绘制箭头（热门航线）
        if ((isHighlighted || route.isPopular) && m_zoomLevel >= 0.7) {
            double angle = std::atan2(end.y() - start.y(), end.x() - start.x());
            double arrowSize = 10 * m_zoomLevel;
            QPointF arrowP1 = end - QPointF(std::cos(angle + M_PI / 6) * arrowSize,
                                            std::sin(angle + M_PI / 6) * arrowSize);
            QPointF arrowP2 = end - QPointF(std::cos(angle - M_PI / 6) * arrowSize,
                                            std::sin(angle - M_PI / 6) * arrowSize);
            
            painter.setBrush(pen.color());
            painter.setPen(Qt::NoPen);
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
    painter.drawRoundedRect(legendX, legendY, 160, 140, 10, 10);
    
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
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(legendX + 25, legendY + 80), 5, 5);
    painter.setPen(QColor(71, 85, 105));
    painter.drawText(legendX + 50, legendY + 85, "城市");
    
    // 统计信息
    painter.drawText(legendX + 10, legendY + 105, 
                    QString("航线总数: %1").arg(m_routes.size()));
    painter.drawText(legendX + 10, legendY + 120, 
                    QString("城市数: %1").arg(m_cities.size()));
    painter.drawText(legendX + 10, legendY + 135, 
                    QString("航班数: %1").arg(m_flights.size()));
}

void MapVisualization::drawZoomIndicator(QPainter& painter)
{
    // 在左下角显示缩放比例
    int x = 20;
    int y = height() - 60;
    
    // 背景
    painter.setBrush(QColor(255, 255, 255, 220));
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    painter.drawRoundedRect(x, y, 120, 40, 8, 8);
    
    // 缩放文字
    painter.setPen(QColor(71, 85, 105));
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);
    
    QString zoomText = QString::fromUtf8("🔍 %1%").arg((int)(m_zoomLevel * 100));
    painter.drawText(x + 10, y + 16, zoomText);
    
    // 操作提示
    font.setPointSize(8);
    font.setBold(false);
    painter.setFont(font);
    painter.setPen(QColor(128, 128, 128));
    painter.drawText(x + 10, y + 32, "Ctrl+滚轮缩放");
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
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    QFontMetrics fm(font);
    
    QStringList lines = tooltip.split('\n');
    int maxWidth = 0;
    for (const QString& line : lines) {
        maxWidth = qMax(maxWidth, fm.horizontalAdvance(line));
    }
    
    QRect textRect(0, 0, maxWidth + 20, lines.size() * fm.height() + 16);
    textRect.moveTopLeft(m_mousePos.toPoint() + QPoint(15, 15));
    
    // 确保提示框不超出窗口
    if (textRect.right() > width()) {
        textRect.moveRight(m_mousePos.x() - 10);
    }
    if (textRect.bottom() > height()) {
        textRect.moveBottom(m_mousePos.y() - 10);
    }
    
    painter.setBrush(QColor(30, 41, 59, 230));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 8, 8);
    
    painter.setPen(Qt::white);
    int yOffset = textRect.top() + fm.ascent() + 8;
    for (const QString& line : lines) {
        painter.drawText(textRect.left() + 10, yOffset, line);
        yOffset += fm.height();
    }
}

void MapVisualization::mouseMoveEvent(QMouseEvent *event)
{
    m_mousePos = event->position();
    
    if (m_isDragging) {
        // 拖拽移动
        QPointF delta = event->position() - m_lastMousePos;
        m_panOffset += delta;
        m_lastMousePos = event->position();
        update();
    } else {
        // 检测悬停城市
        QString oldHovered = m_hoveredCity;
        m_hoveredCity = getCityAtPoint(m_mousePos);
        
        if (oldHovered != m_hoveredCity) {
            update();
        }
    }
}

void MapVisualization::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QString city = getCityAtPoint(event->position());
        if (!city.isEmpty()) {
            emit cityClicked(city);
        } else {
            // 开始拖拽
            m_isDragging = true;
            m_lastMousePos = event->position();
            setCursor(Qt::ClosedHandCursor);
        }
    }
}

void MapVisualization::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDragging) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void MapVisualization::wheelEvent(QWheelEvent *event)
{
    // 需要按住 Ctrl 键才能缩放
    if (event->modifiers() & Qt::ControlModifier) {
        // 获取缩放中心点（鼠标位置）
        QPointF mousePos = event->position();
        
        // 计算缩放因子
        double zoomFactor = 1.0;
        if (event->angleDelta().y() > 0) {
            zoomFactor = 1.15;  // 放大
        } else {
            zoomFactor = 0.87;  // 缩小
        }
        
        double newZoom = m_zoomLevel * zoomFactor;
        newZoom = qBound(m_minZoom, newZoom, m_maxZoom);
        
        if (newZoom != m_zoomLevel) {
            // 保持鼠标位置不变的缩放
            QPointF center(width() / 2.0, height() / 2.0);
            QPointF mouseOffset = mousePos - center - m_panOffset;
            
            double zoomRatio = newZoom / m_zoomLevel;
            m_panOffset = mousePos - center - mouseOffset * zoomRatio;
            
            m_zoomLevel = newZoom;
            emit zoomChanged(m_zoomLevel);
            update();
        }
        
        event->accept();
    } else {
        event->ignore();
    }
}

void MapVisualization::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    calculateScreenPositions();
}

QString MapVisualization::getCityAtPoint(const QPointF& point)
{
    double hitRadius = 15 / m_zoomLevel;  // 根据缩放调整点击范围
    hitRadius = qBound(10.0, hitRadius, 25.0);
    
    for (auto it = m_cities.begin(); it != m_cities.end(); ++it) {
        QPointF cityPos = it->screenPos;
        double distance = std::sqrt(std::pow(point.x() - cityPos.x(), 2) +
                                   std::pow(point.y() - cityPos.y(), 2));
        
        if (distance < hitRadius) {
            return it.key();
        }
    }
    
    return QString();
}
