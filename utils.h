#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDateTime>
#include <qglobal.h>

// 生成16位随机盐值（增强密码安全性）
inline QString generateSalt() {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
    QString salt;

    // 使用QRandomGenerator生成随机索引
    for (int i = 0; i < 16; ++i) {
        // bounded(n) 生成 [0, n-1] 范围内的随机数
        int index = QRandomGenerator::global()->bounded(chars.length());
        salt += chars.at(index);
    }
    return salt;
}

// 密码加密（盐值+密码拼接后SHA-256哈希）
inline QString hashPassword(const QString& password, const QString& salt) {
    QByteArray data = (password + salt).toUtf8();
    return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
}

#endif // UTILS_H
