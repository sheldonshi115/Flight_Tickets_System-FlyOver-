// user.h
#ifndef USER_H
#define USER_H

#include <QString>

enum UserRole {
    User,      // 普通用户
    Admin      // 管理员
};

class User
{
public:
    QString username;
    QString password;
    UserRole role;

    User() = default;
    User(const QString &username, const QString &password, UserRole role)
        : username(username), password(password), role(role) {}
};

#endif // USER_H
