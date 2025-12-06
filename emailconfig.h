#ifndef EMAILCONFIG_H
#define EMAILCONFIG_H

#include <QString>

// QQ 邮箱 SMTP 配置
// 注意：这里使用的是 QQ 邮箱的授权码，不是邮箱密码
// 
// 此配置使用一个发件人邮箱发送给任何收件人邮箱（包括非 QQ 邮箱）
// 
// 获取授权码方法：
// 1. 登录 QQ 邮箱（https://mail.qq.com）
// 2. 点击设置 -> 账户
// 3. 找到"POP3/SMTP/IMAP"选项
// 4. 点击"开启"
// 5. 会生成一个 16 位的授权码

class EmailConfig {
public:
    static const char* SMTP_SERVER;      // QQ 邮箱 SMTP 服务器
    static const int SMTP_PORT;          // QQ 邮箱 SMTP 端口
    static const char* FROM_EMAIL;       // 发件人邮箱（你的 QQ 邮箱，作为系统邮件发送账户）
    static const char* FROM_NAME;        // 发件人显示名称
    static const char* AUTH_CODE;        // QQ 邮箱授权码（此邮箱的授权码）
    static const bool USE_SSL;           // 是否使用 SSL/TLS
};

#endif // EMAILCONFIG_H
