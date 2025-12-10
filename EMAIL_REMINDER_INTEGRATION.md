# 邮箱提醒功能集成说明

## 功能概述

本项目已集成三种邮箱提醒功能：
1. **购票成功提醒** - 用户购票成功后立即发送邮件
2. **积分兑换提醒** - 用户兑换积分奖励后发送邮件
3. **航班起飞提醒** - 航班起飞前30分钟自动发送提醒邮件

## 系统架构

### 核心组件

#### 1. EmailReminder 类 (emailreminder.h / emailreminder.cpp)
- **单例模式**：确保全局只有一个邮件提醒实例
- **三个主要方法**：
  - `sendTicketBookedReminder()` - 发送购票成功提醒
  - `sendPointsRedeemedReminder()` - 发送积分兑换提醒
  - `sendFlightDepartureReminder()` - 发送航班起飞提醒

#### 2. FlightReminderScheduler 类 (flightreminderscheduler.h / flightreminderscheduler.cpp)
- **单例模式**：全局定时检查航班起飞
- **定时器**：每5分钟检查一次即将起飞的航班
- **去重机制**：记录已发送的提醒，避免重复发送
- **主要方法**：
  - `startScheduler()` - 启动定时检查
  - `stopScheduler()` - 停止定时检查
  - `setCheckInterval()` - 设置检查间隔

### 数据库扩展

#### Orders 表新增字段
```sql
ALTER TABLE orders ADD COLUMN account VARCHAR(50);
ALTER TABLE orders ADD FOREIGN KEY (account) REFERENCES users(account);
```

#### Order 类新增属性
- `QString userId` - 关联用户账号
- `setUserId()` / `userId()` - 访问器

### 邮件配置

#### emailconfig.h / emailconfig.cpp
发件人邮箱已配置为：**3524779212@qq.com**

**获取 QQ 邮箱授权码的步骤**：
1. 登录 QQ 邮箱 (https://mail.qq.com)
2. 点击页面右上角的"设置"
3. 选择"账户"标签
4. 找到"POP3/SMTP/IMAP"选项
5. 点击"开启"
6. 使用手机验证身份
7. 生成 16 位授权码
8. 将授权码配置到 `EmailConfig::AUTH_CODE`

## 集成点

### 1. 购票流程集成 (flightmanager.cpp)

**订单创建成功后**（第 ~860 行）：

```cpp
// 设置订单中的用户账号
newOrder.setUserId(m_currentUserAccount);

// ...（其他操作）...

// 发送购票成功邮件提醒
if (!m_currentUserAccount.isEmpty()) {
    UserProfile userProfile = DBManager::instance().loadUserProfile(m_currentUserAccount);
    if (!userProfile.email.isEmpty()) {
        EmailReminder::instance().sendTicketBookedReminder(
            userProfile.email,
            userProfile.nickname.isEmpty() ? m_currentUserAccount : userProfile.nickname,
            flight.flightNumber(),
            flight.departureCity(),
            flight.arrivalCity(),
            flight.departureTime().toString("yyyy-MM-dd HH:mm"),
            newOrder.seatNumber(),
            priceToPay
        );
    }
}
```

### 2. 积分兑换集成 (PointsShopDialog.cpp)

**兑换成功后**（第 ~190 行）：

```cpp
// 发送积分兑换成功邮件提醒
if (!m_account.isEmpty()) {
    UserProfile userProfile = DBManager::instance().loadUserProfile(m_account);
    if (!userProfile.email.isEmpty()) {
        EmailReminder::instance().sendPointsRedeemedReminder(
            userProfile.email,
            userProfile.nickname.isEmpty() ? m_account : userProfile.nickname,
            item.name,
            item.pointsCost
        );
    }
}
```

### 3. 航班起飞提醒集成 (mainwindow.cpp)

**主窗口初始化**（initNotificationSystem 方法）：

```cpp
// 启动航班起飞提醒调度器
FlightReminderScheduler::instance().startScheduler();

// 主窗口析构时停止
FlightReminderScheduler::instance().stopScheduler();
```

## 用户邮箱配置

### 邮箱来源
用户在以下位置配置邮箱：
1. **会员中心** - 个人资料编辑页面
2. **注册时** - 注册表单

### 存储
邮箱存储在 `UserProfile.email` 字段，通过 `DBManager::loadUserProfile()` 访问

### 邮箱验证
当发送邮件时，系统会检查：
- 用户邮箱是否为空
- 邮箱格式是否有效（可选）
- 邮箱是否在数据库中正确保存

## 工作流程

### 购票提醒流程
1. 用户选择座位并确认购票
2. 订单创建成功
3. 系统获取用户邮箱
4. `EmailReminder::sendTicketBookedReminder()` 被调用
5. 邮件通过 SMTP 发送

### 积分兑换提醒流程
1. 用户选择商品并点击兑换
2. 系统验证积分充足
3. 兑换成功，积分被扣除
4. `EmailReminder::sendPointsRedeemedReminder()` 被调用
5. 邮件通过 SMTP 发送

### 航班起飞提醒流程
1. 主窗口启动时，`FlightReminderScheduler` 开始工作
2. 每5分钟检查一次所有订单
3. 查找即将在30分钟内起飞的航班
4. 对于未发送过提醒的订单，发送邮件
5. 记录已发送的提醒，避免重复
6. 清理过期的记录（1天前的航班）

## 邮件内容示例

### 购票成功邮件
```
主题：【飞越订票】购票成功提醒 - CA123

尊敬的 张三 用户，

感谢您的购票！您的购票信息如下：

━━━━━━━━━━━━━━━━━━━━━━━━
航班号：CA123
出发城市：北京
到达城市：上海
出发时间：2024-12-15 10:30
座位号：12A
票价：￥599.00
━━━━━━━━━━━━━━━━━━━━━━━━

请妥善保管您的登机牌，祝您旅途愉快！
```

### 积分兑换邮件
```
主题：【飞越订票】积分兑换成功提醒

尊敬的 张三 用户，

您的积分兑换申请已成功处理！

━━━━━━━━━━━━━━━━━━━━━━━━
兑换项目：10元代金券
消耗积分：900 分
兑换时间：2024-12-15 15:45:30
━━━━━━━━━━━━━━━━━━━━━━━━

感谢您对飞越订票的支持！
```

### 航班起飞提醒邮件
```
主题：【飞越订票】航班起飞提醒 - CA123

尊敬的 张三 用户，

您的航班即将在 30 分钟内起飞，请准时出发！

━━━━━━━━━━━━━━━━━━━━━━━━
航班号：CA123
出发城市：北京
到达城市：上海
起飞时间：2024-12-15 10:30
━━━━━━━━━━━━━━━━━━━━━━━━

温馨提示：
• 请提前 2 小时到达机场
• 携带有效身份证和机票
• 遵守机场安全规定
```

## 技术细节

### SMTP 配置
- **服务器**：smtp.qq.com
- **端口**：465
- **加密方式**：SSL/TLS
- **发件人**：3524779212@qq.com
- **认证方式**：AUTH LOGIN

### 错误处理
- 邮箱为空时的警告日志
- 邮件发送失败时的错误捕获
- 数据库连接失败时的退化处理

### 性能优化
- 单例模式避免重复初始化
- 异步邮件发送（不阻塞 UI）
- 定时检查间隔可配置
- 去重机制避免重复发送

## 配置和调试

### 修改检查间隔
```cpp
// 在 mainwindow.cpp 的 initNotificationSystem 中
FlightReminderScheduler::instance().setCheckInterval(60000);  // 改为1分钟
```

### 调试日志
所有操作都有详细的 qDebug() 输出，可在 Qt Creator 的"应用程序输出"窗口查看

### 测试建议
1. 修改 emailconfig.cpp 中的 FROM_EMAIL 和 AUTH_CODE
2. 创建测试账户并设置邮箱
3. 进行购票、积分兑换操作
4. 查看日志输出
5. 检查邮箱是否收到邮件

## 限制和改进

### 当前限制
- 一个账户只能配置一个邮箱
- 不支持邮件模板自定义
- 不支持邮件抄送/密送

### 可能的改进
1. 支持邮件模板自定义
2. 添加邮件发送历史记录
3. 支持邮件重试机制
4. 添加用户邮件订阅设置
5. 支持多种邮件通知类型的开启/关闭

## 总结

本功能集成为用户提供了三种重要的邮箱提醒：
- ✅ 购票确认通知
- ✅ 积分兑换确认
- ✅ 航班起飞提醒

所有邮件均从 **3524779212@qq.com** 发送，收件人邮箱由用户在会员中心配置。
