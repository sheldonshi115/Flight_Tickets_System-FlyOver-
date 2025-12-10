# 邮箱提醒功能 - 快速启动指南

## 需要配置的内容

### 1️⃣ QQ 邮箱授权码配置

编辑文件 `emailconfig.cpp`，找到这一行：

```cpp
const char* EmailConfig::AUTH_CODE = "";  // 【需要填写】
```

替换为你的 16 位授权码。

**获取授权码步骤**：
1. 访问 https://mail.qq.com
2. 点击"设置" → "账户"
3. 找到"POP3/SMTP/IMAP"，点击"开启"
4. 用手机验证后，复制生成的 16 位授权码
5. 粘贴到上方的代码中

### 2️⃣ 数据库表迁移

如果 orders 表已存在但没有 account 字段，需要在 MySQL 中执行：

```sql
ALTER TABLE orders ADD COLUMN account VARCHAR(50);
ALTER TABLE orders ADD FOREIGN KEY (account) REFERENCES users(account);
```

如果是新建库，系统会自动创建正确的表结构。

### 3️⃣ 用户邮箱配置

用户需要在"会员中心"的个人资料编辑页面填写自己的邮箱地址。邮箱将自动保存到数据库。

## 三种提醒的工作方式

| 提醒类型 | 触发时机 | 邮件来源 | 收件人来源 |
|---------|--------|--------|----------|
| 📌 购票成功 | 用户购票成功后 | 3524779212@qq.com | 用户邮箱 |
| 📌 积分兑换 | 用户兑换积分奖励后 | 3524779212@qq.com | 用户邮箱 |
| 📌 航班起飞 | 航班起飞前30分钟自动检查 | 3524779212@qq.com | 用户邮箱 |

## 核心代码位置

### 购票提醒
- **文件**: `flightmanager.cpp`
- **位置**: 第 ~860 行（订单创建成功后）
- **关键代码**: 
  ```cpp
  EmailReminder::instance().sendTicketBookedReminder(...);
  ```

### 积分兑换提醒
- **文件**: `PointsShopDialog.cpp`
- **位置**: 第 ~190 行（兑换成功后）
- **关键代码**:
  ```cpp
  EmailReminder::instance().sendPointsRedeemedReminder(...);
  ```

### 航班起飞提醒
- **文件**: `mainwindow.cpp`
- **位置**: `initNotificationSystem()` 方法
- **关键代码**:
  ```cpp
  FlightReminderScheduler::instance().startScheduler();
  ```

## 新增文件

| 文件名 | 功能 |
|-------|------|
| `emailreminder.h` | 邮件提醒管理器头文件 |
| `emailreminder.cpp` | 邮件提醒管理器实现 |
| `flightreminderscheduler.h` | 航班起飞提醒调度器头文件 |
| `flightreminderscheduler.cpp` | 航班起飞提醒调度器实现 |
| `EMAIL_REMINDER_INTEGRATION.md` | 详细集成文档 |

## 测试步骤

### 测试购票提醒
1. 用账户登录系统
2. 在"会员中心"填写你的邮箱地址
3. 搜索航班并购票
4. 购票成功后，检查邮箱是否收到邮件

### 测试积分兑换提醒
1. 进入"积分商城"
2. 兑换积分商品
3. 兑换成功后，检查邮箱是否收到邮件

### 测试航班起飞提醒
1. 创建一个出发时间为"当前时间 + 25 分钟"的订单
2. 等待 5 分钟（定时检查间隔）
3. 检查邮箱是否收到起飞提醒

> 💡 提示：可以在 `mainwindow.cpp` 中临时改小检查间隔以加快测试
> ```cpp
> FlightReminderScheduler::instance().setCheckInterval(30000);  // 改为30秒
> ```

## 常见问题

### Q: 为什么没收到邮件？
**A**: 检查以下几点：
- [ ] 邮箱授权码是否正确配置
- [ ] 用户是否填写了邮箱地址
- [ ] 邮箱地址是否正确
- [ ] 查看 Qt Creator 的应用程序输出窗口查看日志

### Q: 收到的邮件为什么进了垃圾箱？
**A**: 这很常见。建议：
- [ ] 在 QQ 邮箱中标记为"不是垃圾"
- [ ] 添加到通讯录
- [ ] 检查邮箱的垃圾清理规则

### Q: 如何修改邮件内容？
**A**: 编辑 `emailreminder.cpp` 中的以下方法：
- `sendTicketBookedReminder()` - 购票邮件
- `sendPointsRedeemedReminder()` - 兑换邮件
- `sendFlightDepartureReminder()` - 起飞邮件

### Q: 如何修改检查间隔？
**A**: 在 `mainwindow.cpp` 的 `initNotificationSystem()` 中添加：
```cpp
FlightReminderScheduler::instance().setCheckInterval(120000);  // 改为2分钟
```

### Q: 如何关闭航班起飞提醒？
**A**: 在 `mainwindow.cpp` 的 `initNotificationSystem()` 中注释掉启动代码：
```cpp
// FlightReminderScheduler::instance().startScheduler();
```

## 调试技巧

### 查看完整日志
1. 打开 Qt Creator
2. 点击"应用程序输出"标签页
3. 查看 `[EmailReminder]` 和 `[FlightReminderScheduler]` 的日志输出

### 强制发送测试邮件
在 Qt Creator 的"调试控制台"中输入：
```cpp
EmailReminder::instance().sendTicketBookedReminder("test@example.com", "测试用户", "CA123", "北京", "上海", "2024-12-15 10:30", "12A", 599.0);
```

### 查看数据库中的订单
```sql
SELECT id, order_num, account, status, depart_time FROM orders WHERE account = '你的账户';
```

## 项目编译

新增文件已添加到 `Flight_Tickets_System-FlyOver-.pro` 中：
```qmake
SOURCES += emailreminder.cpp flightreminderscheduler.cpp
HEADERS += emailreminder.h flightreminderscheduler.h
```

只需按正常方式编译项目即可。

## 快速参考

### EmailReminder 用法
```cpp
// 获取单例
EmailReminder& reminder = EmailReminder::instance();

// 发送购票成功邮件
reminder.sendTicketBookedReminder(
    "user@example.com",      // 收件人邮箱
    "用户名",                // 用户昵称
    "CA123",                 // 航班号
    "北京",                  // 出发城市
    "上海",                  // 到达城市
    "2024-12-15 10:30",     // 出发时间
    "12A",                   // 座位号
    599.0                    // 票价
);

// 发送积分兑换邮件
reminder.sendPointsRedeemedReminder(
    "user@example.com",      // 收件人邮箱
    "用户名",                // 用户昵称
    "10元代金券",           // 商品名称
    900                      // 消耗的积分
);

// 发送航班起飞提醒
reminder.sendFlightDepartureReminder(
    "user@example.com",      // 收件人邮箱
    "用户名",                // 用户昵称
    "CA123",                 // 航班号
    "北京",                  // 出发城市
    "上海",                  // 到达城市
    "2024-12-15 10:30"      // 起飞时间
);
```

### FlightReminderScheduler 用法
```cpp
// 获取单例
FlightReminderScheduler& scheduler = FlightReminderScheduler::instance();

// 启动定时检查（默认每5分钟）
scheduler.startScheduler();

// 修改检查间隔（毫秒）
scheduler.setCheckInterval(60000);  // 改为1分钟

// 停止定时检查
scheduler.stopScheduler();
```

---

**✨ 邮箱提醒功能现已准备就绪！**

如有任何问题，请查看详细的 `EMAIL_REMINDER_INTEGRATION.md` 文档。
