# ✈️ FlyOver - 航班机票预订系统

<p align="center">
  <img src="resources/images/logo.png" alt="FlyOver Logo" width="200">
</p>

<p align="center">
  <strong>一款功能完善的航班机票预订管理系统</strong><br>
  基于 Qt 6 + MySQL 开发，支持 AI 智能客服、会员积分、邮件提醒等丰富功能
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Qt-6.9.3-green.svg" alt="Qt Version">
  <img src="https://img.shields.io/badge/C++-17-blue.svg" alt="C++ Standard">
  <img src="https://img.shields.io/badge/Platform-Windows-lightgrey.svg" alt="Platform">
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License">
</p>

---

## 📖 项目简介

**FlyOver** 是一款功能全面的航班机票预订系统，专为用户提供便捷的机票查询、预订、管理等一站式服务。系统采用现代化的 UI 设计，支持深色/浅色主题切换，并集成了 AI 智能客服，让用户可以通过自然语言完成购票流程。

## ✨ 核心功能

### 🔐 用户系统
- **用户注册/登录** - 支持账号密码登录，记住密码功能
- **忘记密码** - 邮箱验证找回密码
- **用户角色管理** - 区分管理员和普通用户权限（RBAC）
- **个人资料管理** - 头像上传、信息修改

### 🛫 航班管理
- **航班查询** - 按出发地、目的地、日期搜索航班
- **航班列表** - 支持表格视图和卡片视图切换
- **座位选择** - 可视化座位选择界面
- **管理员功能** - 航班的增加、删除、修改操作

### 🎫 订单系统
- **一站式购票** - 查询→选座→支付完整流程
- **订单管理** - 查看、筛选、取消订单
- **登机牌打印** - 生成并打印/导出登机牌（PDF）
- **代金券系统** - 支持使用代金券抵扣票价

### 🤖 AI 智能客服
- **普通模式** - 航班信息咨询、FAQ 问答
- **Agent 模式** - 通过自然语言完成完整购票流程
  - "我想买从北京到上海的机票"
  - "查询明天去广州的航班"
  - 自动识别出发地、目的地、日期

### 💎 会员系统
| 会员等级 | 飞行里程要求 | 专属折扣 |
|---------|-------------|---------|
| 🥉 青铜会员 | 0 km | 无折扣 |
| 🥈 白银会员 | 5,000 km | 95折 |
| 🥇 黄金会员 | 15,000 km | 9折 |
| 💎 铂金会员 | 30,000 km | 85折 |
| 👑 钻石会员 | 50,000 km | 8折 |

- **积分系统** - 购票获得积分
- **积分商城** - 积分兑换代金券
- **飞机币** - 虚拟货币支付系统
- **余额充值** - 充值飞机币

### 📧 邮件提醒系统
- **购票成功提醒** - 购票后自动发送确认邮件
- **积分兑换提醒** - 兑换成功后邮件通知
- **起飞提醒** - 航班起飞前 30 分钟自动邮件提醒

### 📊 数据分析
- **航班统计** - 总航班数、活跃航班数
- **价格分析** - 平均票价、价格分布
- **路线分析** - 热门航线统计
- **数据导出** - 生成统计报告

### 🗺️ 航线地图可视化
- **中国地图** - 直观展示航线分布
- **交互操作** - 支持拖拽和缩放
- **热门航线** - 高亮显示热门航线

### 📱 旅行动态（社区功能）
- **发布动态** - 分享旅行图文
- **点赞评论** - 社区互动功能
- **图片预览** - 大图查看

### 🎨 主题系统（深海之光）
- **浅色模式** - 天空蓝主题 (#42A5F5)
- **深色模式** - 深蓝灰主题 (#121212)
- **磨砂玻璃效果** - 现代化 UI 设计
- **渐变按钮** - 精美的交互体验

### 🌐 多语言支持
- 中文（简体）
- English（开发中）

### 📢 公告系统
- **滚动公告** - 首页滚动显示系统公告
- **系统邮件** - 查看系统通知

---

## 🛠️ 技术栈

| 技术 | 版本 | 说明 |
|-----|------|------|
| **Qt** | 6.9.3 | 跨平台 GUI 框架 |
| **C++** | C++17 | 核心开发语言 |
| **MySQL** | 8.0+ | 关系型数据库 |
| **MinGW** | 64-bit | 编译工具链 |
| **SMTP** | - | 邮件发送服务 |

### Qt 模块使用
```qmake
QT += core gui sql network printsupport widgets
```

---

## 📁 项目结构

```
Flight_Tickets_System-FlyOver-/
├── main.cpp                    # 程序入口
├── mainwindow.cpp/h/ui         # 主窗口
├── login.cpp/h/ui              # 登录界面
├── register.cpp/h/ui           # 注册界面
│
├── # 航班管理模块
├── flight.cpp/h                # 航班数据模型
├── flightmanager.cpp/h/ui      # 航班管理界面
├── flightdialog.cpp/h/ui       # 航班编辑对话框
├── seatdialog.cpp/h/ui         # 座位选择对话框
│
├── # 订单模块
├── order.cpp/h                 # 订单数据模型
├── ordermanager.cpp/h/ui       # 订单管理界面
├── orderconfirmdialog.cpp/h    # 订单确认对话框
├── boardingpass.cpp/h          # 登机牌生成
│
├── # AI 客服模块
├── ai.cpp/h/ui                 # AI 智能客服
│
├── # 会员系统
├── membersystem.cpp/h          # 会员系统核心
├── PointsShopDialog.cpp/h      # 积分商城
├── voucherdialog.cpp/h         # 代金券选择
├── rechargedialog.cpp/h        # 余额充值
│
├── # 邮件系统
├── emailsender.cpp/h           # 邮件发送器
├── emailconfig.cpp/h           # 邮件配置
├── emailreminder.cpp/h         # 邮件提醒管理
├── flightreminderscheduler.cpp/h # 航班提醒调度
│
├── # 数据分析
├── dataanalyticswidget.cpp/h   # 数据分析组件
├── mapvisualization.cpp/h      # 航线地图可视化
│
├── # 用户相关
├── UserProfile.h               # 用户信息结构
├── ProfileDisplayDialog.cpp/h  # 个人资料显示
├── ProfileRefreshDialog.cpp/h  # 个人资料刷新
├── forgotpassworddialog.cpp/h  # 忘记密码
│
├── # 社区功能
├── views/
│   └── travelmoment.cpp/h/ui   # 旅行动态
│
├── # 系统功能
├── dbmanager.cpp/h             # 数据库管理（单例）
├── thememanager.cpp/h          # 主题管理（单例）
├── languagemanager.cpp/h       # 多语言管理
├── notificationmanager.cpp/h   # 通知管理
├── announcementmarquee.cpp/h   # 滚动公告
├── systememaildialog.cpp/h     # 系统邮件
│
├── # 工具类
├── commondefs.cpp/h            # 公共定义
├── styleutils.h                # 样式工具
├── utils.h                     # 通用工具
├── clickablelabel.h            # 可点击标签
│
├── # 资源文件
├── resources/
│   ├── resources.qrc           # Qt 资源文件
│   ├── style.qss               # 全局样式表
│   ├── images/                 # 图片资源
│   └── text/                   # 文本资源
│
├── # 数据库（用户）模型
├── models/
│   ├── user.cpp/h              # 用户模型
│
├── # 构建相关
├── Flight_Tickets_System-FlyOver-.pro  # Qt 项目文件
└── build/                      # 构建输出目录
```

---

## 🚀 快速开始

### 环境要求

- **操作系统**: Windows 10/11
- **Qt**: 6.9.3 或更高版本
- **编译器**: MinGW 64-bit（推荐）或 MSVC
- **数据库**: MySQL 8.0+
- **IDE**: Qt Creator（推荐）

### 安装步骤

#### 1. 克隆项目
```bash
git clone https://github.com/yourusername/Flight_Tickets_System-FlyOver-.git
cd Flight_Tickets_System-FlyOver-
```

#### 2. 配置数据库
在 MySQL 中创建数据库：
```sql
CREATE DATABASE flyover DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
```

修改 `dbmanager.cpp` 中的数据库连接配置：
```cpp
const QString DBManager::DB_NAME = "flyover";
const QString DBManager::DB_HOST = "localhost";
const QString DBManager::DB_USER = "your_username";
const QString DBManager::DB_PWD = "your_password";
const int DBManager::DB_PORT = 3306;
```

#### 3. 配置邮件服务（可选）
修改 `emailconfig.cpp` 配置发件邮箱：
```cpp
// SMTP 服务器配置
m_smtpServer = "smtp.qq.com";
m_smtpPort = 465;
m_senderEmail = "your_email@qq.com";
m_senderPassword = "your_smtp_password";  // 授权码
```

#### 4. 编译运行
```bash
# 使用 Qt Creator 打开 .pro 文件
# 或使用命令行：
qmake Flight_Tickets_System-FlyOver-.pro
mingw32-make
```

---

## 📸 功能截图

> 📝 *截图待添加*

- 登录界面（粒子动画效果）
- 主界面（航班卡片展示）
- 座位选择界面
- AI 智能客服
- 数据分析面板
- 航线地图

---

## 🗺️ 开发计划

- [x] 基础用户系统
- [x] 航班管理功能
- [x] 订单系统
- [x] AI 智能客服
- [x] 会员积分系统
- [x] 邮件提醒功能
- [x] 数据分析模块
- [x] 主题系统
- [ ] 多语言完整支持
- [ ] 移动端适配
- [ ] 在线支付集成
- [ ] 航班实时状态

---

## 🤝 贡献指南

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 提交 Pull Request

---

## 📄 开源协议

本项目采用 [MIT License](LICENSE) 开源协议。

---

## 📞 联系方式

- **项目主页**: [GitHub Repository](https://github.com/sheldonshi115/Flight_Tickets_System-FlyOver-)
- **问题反馈**: [Issues](https://github.com/sheldonshi115/Flight_Tickets_System-FlyOver-/issues)

---

<p align="center">
  Made with ❤️ by FlyOver Team
</p>
