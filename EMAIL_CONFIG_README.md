# 邮箱配置说明

## 快速开始

本项目的"忘记密码"功能使用 QQ 邮箱的 SMTP 服务发送验证码。可以发送给**任何邮箱地址**（QQ邮箱、Gmail、Outlook、自建邮箱等）。

### 工作原理

- **发件人**: 配置的 QQ 邮箱（用作系统邮件发送账户）
- **收件人**: 任何邮箱地址（包括 QQ邮箱、163邮箱、Gmail 等）
- **传输**: 通过 SMTP 和 SSL/TLS 加密传输

### 1. 获取 QQ 邮箱授权码

1. 登录你的 QQ 邮箱：https://mail.qq.com
2. 点击右上角的**设置**按钮
3. 选择左侧菜单的**账户**
4. 在"POP3/SMTP/IMAP"部分，点击**开启**
5. 使用手机 QQ 或短信验证身份
6. 验证后，系统会生成一个 **16 位的授权码**
7. 复制这个授权码

### 2. 配置应用中的邮箱信息

打开项目中的 `emailconfig.cpp` 文件，找到以下行：

```cpp
const char* EmailConfig::FROM_EMAIL = "your_qq_email@qq.com";     // 替换为你的 QQ 邮箱（发件人）
const char* EmailConfig::FROM_NAME = "飞越订票系统";              // 发件人显示名称
const char* EmailConfig::AUTH_CODE = "your_auth_code";            // 替换为你的授权码
```

替换为：
- `your_qq_email@qq.com` → 你的 QQ 邮箱地址（例如：`123456789@qq.com`）
- `your_auth_code` → 你刚才获取的 16 位授权码

### 3. 重新编译项目

完成配置后，重新编译项目即可。

## 测试功能

1. 运行应用程序
2. 在登录界面点击"忘记密码？"
3. 输入任何邮箱地址（**注意**：该邮箱必须已在系统中注册）
4. 点击"发送验证码"
5. 检查对应邮箱的收件箱，应该会收到验证码邮件

## 支持的邮箱类型

✅ QQ 邮箱 (qq.com)
✅ QQ 企业邮箱 (exmail.qq.com)  
✅ 163 邮箱 (163.com)
✅ 126 邮箱 (126.com)
✅ 新浪邮箱 (sina.com)
✅ Gmail (gmail.com)
✅ Outlook/Hotmail (outlook.com)
✅ 其他支持标准 SMTP 的邮箱

## 常见问题

### Q: 只能发送给 QQ 邮箱吗？
**A:** 不是。系统使用 QQ 邮箱作为发件人账户，但可以发送给任何邮箱地址。

### Q: 提示"邮件发送失败"
**A:** 请检查：
1. 邮箱地址和授权码配置是否正确
2. 网络连接是否正常
3. 防火墙是否阻止了 SMTP 端口 465
4. 输入的邮箱地址是否已在系统中注册

### Q: 无法连接到 SMTP 服务器
**A:** 
1. 确保网络连接正常
2. 确认防火墙或代理没有阻止端口 465
3. 检查 QQ 邮箱是否已开启 POP3/SMTP/IMAP
4. 确认授权码未过期（某些情况下需要重新获取）

### Q: 收不到验证码邮件
**A:**
1. 检查目标邮箱的垃圾邮件文件夹
2. 确认目标邮箱地址拼写无误
3. 确认该邮箱已在系统中注册
4. 尝试重新发送，QQ 邮箱每小时有发送限制

## 技术细节

- **SMTP 服务器**: smtp.qq.com
- **SMTP 端口**: 465 (SSL/TLS)
- **验证方式**: AUTH LOGIN
- **字符编码**: UTF-8 with Base64 encoding
- **验证码有效期**: 5 分钟
- **验证码长度**: 6 位数字

## 安全提示

⚠️ **重要**: 不要在公开的代码仓库中提交真实的邮箱地址和授权码。如果需要提交到 Git，请：

1. 修改 `emailconfig.cpp` 使用环境变量：
```cpp
#include <cstdlib>
const char* EmailConfig::FROM_EMAIL = std::getenv("FLYOVER_EMAIL");
const char* EmailConfig::AUTH_CODE = std::getenv("FLYOVER_AUTH_CODE");
```

2. 在部署时设置环境变量
3. 将 `emailconfig.cpp` 添加到 `.gitignore`

## 联系我们

如有问题，请联系开发团队。
