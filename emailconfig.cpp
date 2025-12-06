#include "emailconfig.h"

// ========== 重要：请按以下步骤配置你的 QQ 邮箱 ==========
// 此邮箱用作系统邮件的发件人账户，可以发送给任何收件人（包括非 QQ 邮箱）
// 
// 1. 登录 QQ 邮箱：https://mail.qq.com
// 2. 点击页面右上角的"设置"
// 3. 选择"账户"标签
// 4. 找到"POP3/SMTP/IMAP"选项
// 5. 点击"开启"按钮
// 6. 按照提示，使用手机进行身份验证
// 7. 验证后会生成一个 16 位的授权码
// 8. 将授权码复制到下面的 AUTH_CODE
// ==============================================

// 替换为你的实际配置
const char* EmailConfig::SMTP_SERVER = "smtp.qq.com";
const int EmailConfig::SMTP_PORT = 465;
const char* EmailConfig::FROM_EMAIL = "";              // 替换为你的 QQ 邮箱（发件人）
const char* EmailConfig::FROM_NAME = "飞越订票系统";                        // 发件人显示名称
const char* EmailConfig::AUTH_CODE = "";                      // 替换为你的授权码
const bool EmailConfig::USE_SSL = true;

