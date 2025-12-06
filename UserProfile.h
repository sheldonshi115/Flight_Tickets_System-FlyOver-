#ifndef USERPROFILE_H
#define USERPROFILE_H

#include <QString>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
// 定义性别枚举
enum Gender {
    Male = 0,
    Female = 1,
    Unknown = 2
};

// 用户信息结构体
struct UserProfile {
    QString account;    // 账号
    QString nickname;   // 昵称
    Gender gender;      // 性别
    QString phone;      // 手机号
    QString email;      // 邮箱
    QPixmap avatar;     // 头像图片
};
static QPixmap getRoundPixmap(const QPixmap& src, QSize size) {
    if (src.isNull()) return QPixmap();

    // 1. 缩放图片（平滑缩放，填充目标大小）
    QPixmap result(size);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // 2. 绘制圆形路径
    QPainterPath path;
    path.addEllipse(0, 0, size.width(), size.height());
    painter.setClipPath(path);

    // 3. 将原图画入圆形区域
    // 使用 KeepAspectRatioByExpanding 确保图片填满圆形，不会有白边
    painter.drawPixmap(0, 0, src.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    return result;
}
#endif // USERPROFILE_H

