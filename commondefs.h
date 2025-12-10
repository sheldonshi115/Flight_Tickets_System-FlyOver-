// commondefs.h
#ifndef COMMONDEFS_H
#define COMMONDEFS_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QtMath>

// 座位状态枚举（全局共享）
enum SeatState {
    Available, // 可选
    Sold,      // 已售
    Selected   // 已选
};

// 座位数据结构体（全局共享）
struct SeatData {
    QString seatId;    // 座位号（如"1A"）
    SeatState state;
    bool isWindow;     // 靠窗
    bool isAisle;      // 靠过道
};

// 航班信息结构体（全局共享）
struct FlightInfo {
    QString flightNumber;
    QString departureCity;
    QString arrivalCity;
    QString dateTime;
    QVector<SeatData> allSeats; // 所有座位数据
};

// 计算两个城市间的飞行距离（公里）
inline double calculateFlightDistance(const QString& city1, const QString& city2) {
    // 城市坐标结构
    struct CityCoord {
        double lat;
        double lon;
    };
    
    // 中国主要城市坐标（纬度，经度）
    static QMap<QString, CityCoord> cityCoords = {
        {"北京", {39.9042, 116.4074}},
        {"上海", {31.2304, 121.4737}},
        {"广州", {23.1291, 113.2644}},
        {"深圳", {22.5431, 114.0579}},
        {"成都", {30.5728, 104.0668}},
        {"杭州", {30.2741, 120.1551}},
        {"重庆", {29.5630, 106.5516}},
        {"西安", {34.3416, 108.9398}},
        {"苏州", {31.2989, 120.5853}},
        {"武汉", {30.5928, 114.3055}},
        {"南京", {32.0603, 118.7969}},
        {"天津", {39.3434, 117.3616}},
        {"郑州", {34.7466, 113.6253}},
        {"长沙", {28.2282, 112.9388}},
        {"沈阳", {41.8057, 123.4328}},
        {"青岛", {36.0671, 120.3826}},
        {"大连", {38.9140, 121.6147}},
        {"厦门", {24.4798, 118.0894}},
        {"哈尔滨", {45.8038, 126.5340}},
        {"昆明", {25.0406, 102.7129}},
        {"乌鲁木齐", {43.8256, 87.6168}}
    };
    
    if (!cityCoords.contains(city1) || !cityCoords.contains(city2)) {
        return 500.0; // 默认距离
    }
    
    CityCoord coord1 = cityCoords[city1];
    CityCoord coord2 = cityCoords[city2];
    
    // 使用 Haversine 公式计算球面距离
    const double R = 6371.0; // 地球半径（公里）
    double lat1 = qDegreesToRadians(coord1.lat);
    double lat2 = qDegreesToRadians(coord2.lat);
    double dLat = qDegreesToRadians(coord2.lat - coord1.lat);
    double dLon = qDegreesToRadians(coord2.lon - coord1.lon);
    
    double a = qSin(dLat/2) * qSin(dLat/2) +
               qCos(lat1) * qCos(lat2) *
               qSin(dLon/2) * qSin(dLon/2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1-a));
    double distance = R * c;
    
    return qRound(distance); // 四舍五入到整数公里
}

#endif // COMMONDEFS_H
