#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGlobal>
#include <QTextStream>
#include <utility>
#include "views/travelmoment.h"
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QProcess>
#include "ai.h"
#include <ordermanager.h>
#include "dataanalyticswidget.h"
#include "ProfileDisplayDialog.h"
#include "thememanager.h"
#include "languagemanager.h"
#include "notificationmanager.h"
#include "membersystem.h"
#include "mapvisualization.h"
#include "dbmanager.h"
#include "PointsShopDialog.h"
#include "flightreminderscheduler.h"
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QMessageBox>
#include <QInputDialog>
#include "rechargedialog.h"
#include "systememaildialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_flightManager(nullptr),
    m_aiquery(nullptr),
    m_isAdmin(false),
    m_orderManager(nullptr),
    m_dataAnalytics(nullptr)
{
    ui->setupUi(this);

    if (ui->actionProfile) {
        connect(ui->actionProfile, &QAction::triggered, this, &MainWindow::on_actionUserinfo_triggered);
    }
    // 有时 UI 文件中个人信息菜单项的对象名为 "actions"（生成的 ui_mainwindow.h 使用该名字），
    // 因此也要连接它以确保菜单点击能打开个人信息对话框。
    //if (ui->actions) {
    //    connect(ui->actions, &QAction::triggered, this, &MainWindow::on_actionUserinfo_triggered);
    //}
    // 原有信号槽（保留）
    connect(ui->btnHome, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageHome);
    });
    connect(ui->travelbutton, &QPushButton::clicked, this, &MainWindow::onTravelButtonClicked);
    connect(ui->btnFlightQuery, &QPushButton::clicked, this, &MainWindow::on_btnFlightQuery_clicked);
    connect(ui->btnOrders, &QPushButton::clicked, this, &MainWindow::on_btnOrders_clicked);
    connect(ui->actionViewOrders, &QAction::triggered, this, &MainWindow::on_btnOrders_clicked);
    connect(ui->btnProfile,&QPushButton::clicked,this,&MainWindow::clicked_btnProfile);
    // 连接侧边栏"我的消息"按钮
    connect(ui->btnMessages, &QPushButton::clicked, this, [this]() {
        SystemEmailDialog* dlg = new SystemEmailDialog(m_appUser.account, this);
        dlg->exec();
        dlg->deleteLater();
    });

    // 数据分析按钮连接
    connect(ui->btnDataAnalytics, &QPushButton::clicked, this, &MainWindow::on_btnDataAnalytics_clicked);
    
    // btnProfile 现在代表会员中心，显示全面的会员信息
    // 【新增】提前初始化OrderManager（确保信号连接时已存在）
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
        qDebug() << "[MainWindow] OrderManager 已初始化";
    }

    setWindowTitle(QString::fromUtf8("航班票务系统 - FlyOver"));
    
    // 初始化用户信息（在调用 initHomePage 之前）
    m_appUser.account = "";
    m_appUser.nickname = "用户";
    m_appUser.gender = Gender::Unknown;
    m_appUser.phone = "";
    m_appUser.email = "";
    m_appUser.avatar = QPixmap(":/images/default_avatar.jpg");
    
    // 默认为普通用户，但不调用 setIsAdmin（由 login.cpp 调用）
    m_isAdmin = false;
    
    // 强制应用浅色主题（不受系统深色模式影响）
    ThemeManager::instance().forceLightTheme();
    
    // 初始化主页面（立即初始化，显示欢迎界面）
    initHomePage();

    // 连接会员系统的变更信号，确保当当前登录用户的余额/积分/里程/等级发生变化时，主页面会同时刷新显示
    {
        MemberSystem &member = MemberSystem::instance();
        // Prefer listening to the aggregated signal to ensure a single, synchronized refresh
        connect(&member, &MemberSystem::memberInfoChanged, this, [this](const QString &userId) {
            if (userId == m_appUser.account) {
                QMetaObject::invokeMethod(this, "updateMemberInfo", Qt::QueuedConnection);
            }
        });
        // Keep individual signals (optional) for other parts of UI that may want fine-grained updates
        connect(&member, &MemberSystem::balanceChanged, this, [](const QString &/*userId*/, double) {
            // no-op here; main window relies on memberInfoChanged for full refresh
        });
    }
    
    // 初始化语言切换和通知系统（延迟初始化避免启动时阻塞）
    QTimer::singleShot(500, this, [this]() {
        initLanguageSwitch();
        initNotificationSystem();
    });

    // 创建浮动小球 AI 按钮（初始在主窗口左侧隐藏）
    m_floatingAIButton = new QPushButton(this);
    m_floatingAIButton->setObjectName("floatingAIButton");
    m_floatingAIButton->setFixedSize(40, 40); // 小球尺寸
    m_floatingAIButton->setToolTip("AI Assistant");
    m_floatingAIButton->setFlat(true);
    m_floatingAIButton->setStyleSheet("background: transparent; border: none;");
    // 修改：绘制蓝色圆，中心绘制白色小飞机✈
    const int size = 40;
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    // 蓝色圆
    QColor blue(33,150,243); // #2196F3
    painter.setBrush(blue);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);
    // 绘制白色小飞机✈ emoji
    painter.setPen(QColor(255, 255, 255));
    QFont font = painter.font();
    font.setPixelSize(20);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, QString::fromUtf8("✈"));
    painter.end();

    QPixmap finalPixmap = pix;
    m_floatingAIButton->setIcon(QIcon(finalPixmap));
    m_floatingAIButton->setIconSize(finalPixmap.size());
    // 初始位置置于左侧大部分隐藏状态（相对于主窗口）
    int initBtnW = m_floatingAIButton->width();
    int initX = - (initBtnW - 8);
    int initY = qMax(40, (this->height() / 2) - initBtnW/2);
    m_floatingAIButton->move(initX, initY);
    m_floatingAIButton->hide();

    // 动画对象（用于滑出/滑入）
    m_floatingAnim = new QPropertyAnimation(m_floatingAIButton, "pos", this);
    m_floatingAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_floatingAnim->setDuration(320);

    // 安装事件过滤器：同时在应用级安装以捕获全局鼠标移动（只要鼠标靠近主窗口左侧就弹出）
    this->setMouseTracking(true);
    this->installEventFilter(this);
    qApp->installEventFilter(this);
    m_floatingAIButton->setMouseTracking(true);
    m_floatingAIButton->installEventFilter(this);

    // 点击时复用原来的 AI 弹窗逻辑
    connect(m_floatingAIButton, &QPushButton::clicked, this, &MainWindow::on_btnAIService_clicked);
}

void MainWindow::on_btnAIService_clicked(){
    // 创建为顶级窗口（传入 nullptr），以保留窗口装饰（最大化/最小化/关闭按钮）
    AIQueryWidget *aiq = new AIQueryWidget(nullptr);
    // 将主窗口设置为 owner（用于动画定位回到该窗口的 AI 按钮）
    aiq->setOwnerWindow(this);
    // 将当前主窗口保存的用户头像传递给AI对话窗口，以在用户消息前显示
    aiq->setUserAvatar(m_appUser.avatar);
    
    // 传递FlightManager引用和当前用户账号（Agent模式需要）
    if (m_flightManager) {
        aiq->setFlightManager(m_flightManager);
    }
    if (!m_appUser.account.isEmpty()) {
        aiq->setCurrentUser(m_appUser.account);
    }
    
    // 使用带动画的显示方法
    aiq->showWithAnimation();
}

void MainWindow::on_btnRecharge_clicked()
{
    if (m_appUser.account.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("未登录"), QString::fromUtf8("请先登录后再充值。"));
        return;
    }
    // 使用定制化充值对话框（更像 QQ Q 币的界面）
    RechargeDialog dlg(m_appUser.account, this);
    connect(&dlg, &RechargeDialog::rechargeSucceeded, this, [this](double amount){
        Q_UNUSED(amount);
        updateMemberInfo();
    });
    dlg.exec();
}
void MainWindow::set_account(QString acc){
    m_appUser.account = acc;
}

void MainWindow::setUserProfile(const UserProfile& profile) {
    m_appUser = profile;
    qDebug() << "[MainWindow] setUserProfile 被调用"
             << "account=" << profile.account
             << "nickname=" << profile.nickname
             << "phone=" << profile.phone
             << "email=" << profile.email
             << "role=" << (profile.isAdmin() ? "Admin" : "User");
    
    updateWelcomeLabel();
    
    // 更新主页面显示的用户信息
    updateMemberInfo();
    
    // 注意：不在这里调用 setIsAdmin，由 login.cpp 中调用
    // setIsAdmin(profile.isAdmin());
}
MainWindow::~MainWindow()
{
    qDebug() << "[MainWindow] 析构函数被调用";
    
    // 【新增】停止航班提醒调度器
    FlightReminderScheduler::instance().stopScheduler();
    qDebug() << "[MainWindow] 航班起飞提醒调度器已停止";
    
    // 安全删除浮动按钮和动画
    if (m_floatingAnim) {
        m_floatingAnim->stop();
        delete m_floatingAnim;
        m_floatingAnim = nullptr;
    }
    if (m_floatingAIButton) {
        delete m_floatingAIButton;
        m_floatingAIButton = nullptr;
    }
    
    // stackedWidget 会自动清理其子控件
    // m_flightManager、m_orderManager、m_dataAnalytics 都是 stackedWidget 的子级
    // 不需要手动删除
    
    delete ui;
}

// 实现权限设置函数 - 根据用户角色显示不同界面
void MainWindow::setIsAdmin(bool isAdmin)
{
    m_isAdmin = isAdmin;
    
    qDebug() << "[RBAC] setIsAdmin 被调用, isAdmin=" << isAdmin;
    
    // 根据角色调整界面 - RBAC 路由逻辑
    if (isAdmin) {
        // ========== 管理员模式 (ADMIN) ==========
        // 路由: /admin/dashboard - 管理员控制台
        setWindowTitle("航班票务系统 - 管理员控制台");
        
        // 显示管理员专属功能
        if (ui->btnDataAnalytics) {
            ui->btnDataAnalytics->setVisible(true);
            ui->btnDataAnalytics->setText("📊 数据分析");
        }
        
        // 航班管理菜单显示（仅管理员）
        if (ui->menuFlights) {
            ui->menuFlights->menuAction()->setVisible(true);
        }
        if (ui->actionFlightManager) {
            ui->actionFlightManager->setVisible(true);
        }
        
        // 安全检查：确保 sideBar 存在
        if (ui->sideBar) {
            // 管理员侧边栏按钮样式 - 与用户模式一致（黑色文字）
            QString adminSidebarStyle = R"(
                QPushButton {
                    background-color: transparent;
                    color: #333333;
                    border: none;
                    border-radius: 8px;
                    text-align: left;
                    padding: 12px 20px;
                    font-size: 14px;
                }
                QPushButton:hover {
                    background-color: rgba(59, 130, 246, 0.1);
                }
                QPushButton:pressed {
                    background-color: rgba(59, 130, 246, 0.2);
                }
            )";
            
            // 应用管理员样式到侧边栏
                    QList<QPushButton*> sideButtons = ui->sideBar->findChildren<QPushButton*>();
                    for (QPushButton *btn : std::as_const(sideButtons)) {
                        if (btn) btn->setStyleSheet(adminSidebarStyle);
                    }
        }
        
        qDebug() << "[RBAC] 路由: /admin/dashboard - 管理员模式";
        
    } else {
        // ========== 普通用户模式 (GENERAL) ==========
        // 路由: /user/query - 用户购票界面
        setWindowTitle("航班票务系统 - FlyOver");
        
        // 隐藏管理员专属功能
        if (ui->btnDataAnalytics) {
            ui->btnDataAnalytics->setVisible(false);
        }
        
        // 隐藏航班管理菜单（普通用户不能增删改航班）
        if (ui->menuFlights) {
            ui->menuFlights->menuAction()->setVisible(false);
        }
        if (ui->actionFlightManager) {
            ui->actionFlightManager->setVisible(false);
        }
        
        // 普通用户侧边栏按钮样式 - 天空青主题
        QString userSidebarStyle = R"(
            QPushButton {
                background-color: transparent;
                color: #424242;
                border: none;
                border-radius: 8px;
                text-align: left;
                padding: 12px 20px;
                font-size: 14px;
            }
            QPushButton:hover {
                background-color: rgba(52, 152, 219, 0.1);
            }
            QPushButton:pressed {
                background-color: rgba(52, 152, 219, 0.2);
            }
        )";
        
        // 安全检查：确保 sideBar 存在
        if (ui->sideBar) {
            // 应用普通用户样式到侧边栏
            QList<QPushButton*> sideButtons = ui->sideBar->findChildren<QPushButton*>();
            for (QPushButton *btn : std::as_const(sideButtons)) {
                if (btn) btn->setStyleSheet(userSidebarStyle);
            }
        }
        
        qDebug() << "[RBAC] 路由: /user/query - 普通用户模式";
    }
    
    // 自动跳转到对应的默认页面
    navigateToDefaultPage();
}

// 根据角色导航到默认页面
void MainWindow::navigateToDefaultPage()
{
    qDebug() << "[RBAC] navigateToDefaultPage 被调用, isAdmin=" << m_isAdmin;
    
    if (m_isAdmin) {
        // 管理员默认进入航班管理页面 (表格视图)
        // 安全检查：确保控件存在
        if (ui->actionFlightManager) {
            qDebug() << "[RBAC] 管理员模式，跳转到航班管理页面";
            on_actionFlightManager_triggered();
        } else {
            qDebug() << "[RBAC] 警告：actionFlightManager 不存在，跳转到主页";
            ui->stackedWidget->setCurrentWidget(ui->pageHome);
        }
    } else {
        // 普通用户默认进入主页（修正：登录后应显示主页，而不是直接进入航班查询）
        qDebug() << "[RBAC] 普通用户模式，跳转到主页";
        if (ui && ui->pageHome) {
            ui->stackedWidget->setCurrentWidget(ui->pageHome);
        }
    }
}

void MainWindow::onTravelButtonClicked()
{
    // 原有旅行动态逻辑（保留）
    for (int i = 0; i < ui->stackedWidget->count(); ++i) {
        QWidget *w = ui->stackedWidget->widget(i);
        TravelMoment *tm = qobject_cast<TravelMoment*>(w);
        if (tm) {
            ui->stackedWidget->setCurrentWidget(tm);
            return;
        }
    }
    TravelMoment *tm = new TravelMoment(ui->stackedWidget); // 父对象设为stackedWidget
    tm->setObjectName("travelMomentWidget");
    ui->stackedWidget->addWidget(tm);
    ui->stackedWidget->setCurrentWidget(tm);
}

void MainWindow::on_actionFlightManager_triggered()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);
        
        // 设置当前用户
        m_flightManager->setCurrentUser(m_appUser.account);

        // 【新增】连接信号（管理员模式购票也需要同步订单）
        // 确保 m_orderManager 存在后再连接
        if (m_orderManager) {
            connect(m_flightManager, &FlightManager::orderCreated,
                    m_orderManager, &OrderManager::refreshOrderList);
            // 同步刷新主窗口的会员信息，确保购票后主页面余额/积分立即更新
                    connect(m_flightManager, &FlightManager::orderCreated,
                        this, &MainWindow::updateMemberInfo,
                        Qt::QueuedConnection);
        }
    }
    if (m_flightManager) {
        m_flightManager->setAdminMode(true); // 管理员模式
        ui->stackedWidget->setCurrentWidget(m_flightManager);
    }
}
void MainWindow::on_btnFlightQuery_clicked()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);
        
        // 设置当前用户
        m_flightManager->setCurrentUser(m_appUser.account);

        // 【新增】连接FlightManager的订单创建信号到OrderManager的刷新方法
        // 确保 m_orderManager 存在后再连接
        if (m_orderManager) {
            connect(m_flightManager, &FlightManager::orderCreated,
                    m_orderManager, &OrderManager::refreshOrderList);
            // 同步刷新主窗口的会员信息，确保购票后主页面余额/积分立即更新
                    connect(m_flightManager, &FlightManager::orderCreated,
                        this, &MainWindow::updateMemberInfo,
                        Qt::QueuedConnection);
        }
    }
    if (m_flightManager) {
        m_flightManager->setAdminMode(false); // 普通查询模式
        ui->stackedWidget->setCurrentWidget(m_flightManager);
    }
}
void MainWindow::on_btnOrderManager_clicked() // 假设主窗口有"订单管理"按钮
{
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
        qDebug() << "[MainWindow] OrderManager 延迟创建";
    }
    if (m_orderManager) {
        ui->stackedWidget->setCurrentWidget(m_orderManager);
    }
}
void MainWindow::on_btnOrders_clicked()
{
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
        qDebug() << "[MainWindow] OrderManager 延迟创建";
    }
    if (m_orderManager) {
        ui->stackedWidget->setCurrentWidget(m_orderManager);
    }
}

// 数据分析按钮点击事件
void MainWindow::on_btnDataAnalytics_clicked()
{
    if (!m_dataAnalytics) {
        m_dataAnalytics = new DataAnalyticsWidget(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_dataAnalytics);
        qDebug() << "[MainWindow] DataAnalyticsWidget 已创建";
    }
    if (m_dataAnalytics) {
        ui->stackedWidget->setCurrentWidget(m_dataAnalytics);
    }
}

// 新增：退出登录逻辑
void MainWindow::on_actionLogout_triggered()
{
    // 只弹出一次确认框
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "退出登录", "确定要退出当前账号吗？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
    if (reply != QMessageBox::Yes) {
        return;
    }

    qDebug() << "[MainWindow] 用户退出登录，准备重启应用程序";

    // 获取当前可执行文件路径和参数
    QString program = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    arguments.removeFirst(); // 移除程序路径本身
    
    qDebug() << "[MainWindow] 准备重启: " << program;
    
    // 关闭主窗口
    this->close();
    
    // 退出当前应用程序并重新启动
    // exitCode = 1000 表示需要重启
    QApplication::exit(1000);
}
void MainWindow::clicked_btnProfile(){
    qDebug() << "clicked_btnProfile: m_appUser.account=" << m_appUser.account;
    
    // 1. 创建展示窗口（指定 this 为父对象，由主窗口管理内存）
    ProfileDisplayDialog dlg(this);

    // 2. 将主窗口持有的数据传给弹窗
    dlg.updateDisplay(m_appUser);

    // 3. 以模态方式运行（阻塞主窗口，直到弹窗关闭）
    dlg.exec();

    // 4. 窗口关闭后，数据可能在里面被修改了，我们需要取回最新数据
    // 这样下次再打开时，显示的就是修改后的新数据了
    m_appUser = dlg.getCurrentProfile();

    // 5. (可选) 如果主窗口也有显示头像/昵称的地方，这里记得刷新一下主窗口的UI
    // ui->lblMainAvatar->setPixmap(m_appUser.avatar);
}

void MainWindow::on_actionUserinfo_triggered()
{
    // 打开个人信息对话框（侧边菜单触发）
    ProfileDisplayDialog dlg(this);
    dlg.updateDisplay(m_appUser);
    if (dlg.exec() == QDialog::Accepted) {
        // 如果用户在对话框中编辑并保存，取回最新数据并刷新主界面
        m_appUser = dlg.getCurrentProfile();
        updateMemberInfo();
    }
}

void MainWindow::on_actionAbout_triggered()
{
    // 严格组织关于信息，包含必要要素
    const QString systemName = "航班票务系统";
    const QString version = "v1.2.1";
    const QString releaseDate = "2025-12";
    const QString description = "本系统提供航班查询、机票预订、AI助手服务等功能，\n"
                                "支持管理员与普通用户两种操作模式，\n"
                                "旨在为用户提供便捷的航空出行解决方案。";
    const QString copyright = "© 2025 航班票务系统开发团队 版权所有";
    const QString contact = "技术支持：support@flightsystem.com";

    // 组合信息文本，使用 QTextStream 避免多个 chained .arg() 带来的临时对象
    QString aboutText;
    QTextStream ss(&aboutText);
    ss << systemName << "\n\n"
       << "版本：" << version << "\n"
       << "发布日期：" << releaseDate << "\n\n"
       << description << "\n\n"
       << copyright << "\n\n"
       << contact;

    // 使用Qt标准关于对话框，父窗口为当前MainWindow，确保模态显示
    QMessageBox::about(this,
                       tr("关于%1").arg(systemName),  // 标题国际化（tr函数支持多语言）
                       aboutText);
}
void MainWindow::slideOutFloatingButton() {
    if (!m_floatingAIButton || !m_floatingAnim) return;
    int btnW = m_floatingAIButton->width();
    int x = 8; // visible offset from left client edge
    int y = qMax(40, (this->height() / 2) - btnW/2);
    QPoint endPos(x, y);
    m_floatingAIButton->show();
    m_floatingAnim->stop();
    m_floatingAnim->setStartValue(m_floatingAIButton->pos());
    m_floatingAnim->setEndValue(endPos);
    m_floatingAnim->start();
    m_floatingVisible = true;
}

void MainWindow::slideInFloatingButton() {
    if (!m_floatingAIButton || !m_floatingAnim) return;
    int btnW = m_floatingAIButton->width();
    int x = - (btnW - 8); // hide most of button off left
    int y = qMax(40, (this->height() / 2) - btnW/2);
    QPoint endPos(x, y);
    m_floatingAnim->stop();
    m_floatingAnim->setStartValue(m_floatingAIButton->pos());
    m_floatingAnim->setEndValue(endPos);
    // disconnect previous finished connections to avoid multiple hides
    QObject::disconnect(m_floatingAnim, &QPropertyAnimation::finished, nullptr, nullptr);
    connect(m_floatingAnim, &QPropertyAnimation::finished, this, [this]() {
        if (!m_floatingVisible && !m_buttonHovered) {
            if (m_floatingAIButton) m_floatingAIButton->hide();
        }
    });
    m_floatingAnim->start();
    m_floatingVisible = false;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    // 捕获全局鼠标移动事件（安装在 qApp 上），使用全局坐标判断与主窗口左侧的距离
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me) {
            QPoint global = me->globalPosition().toPoint();
            int windowLeft = this->mapToGlobal(QPoint(0,0)).x();
            int xThreshold = 48; // 横向阈值
            int dx = global.x() - windowLeft; // 鼠标相对窗口左边的横向偏移

            bool nearLeft = (dx <= xThreshold && dx >= -xThreshold);
            if (nearLeft) {
                if (!m_floatingVisible) slideOutFloatingButton();
            } else {
                if (m_floatingVisible && !m_buttonHovered) slideInFloatingButton();
            }
        }
        // 继续让事件传递
        return false;
    }

    // Hover enter/leave on the floating button
    if (watched == m_floatingAIButton) {
        if (event->type() == QEvent::Enter) {
            m_buttonHovered = true;
            if (!m_floatingVisible) slideOutFloatingButton();
            return false;
        } else if (event->type() == QEvent::Leave) {
            m_buttonHovered = false;
            // hide if mouse is away from left edge
            QPoint global = QCursor::pos();
            QPoint rel = this->mapFromGlobal(global);
            int threshold = 24;
            if (rel.x() > threshold) slideInFloatingButton();
            return false;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

// 初始化语言切换功能
void MainWindow::initLanguageSwitch()
{
    // 创建小飞机图标标签
    m_planeIconLabel = new QLabel(this);
    m_planeIconLabel->setText("✈️");
    m_planeIconLabel->setStyleSheet(R"(
        QLabel {
            font-size: 24px;
            padding: 5px;
        }
    )");
    m_planeIconLabel->setToolTip("FlyOver Airlines");
    
    // 创建语言切换下拉框
    m_languageCombo = new QComboBox(this);
    m_languageCombo->addItem("🇨🇳 中文", static_cast<int>(Language::Chinese));
    m_languageCombo->addItem("🇺🇸 English", static_cast<int>(Language::English));
    m_languageCombo->setCurrentIndex(0);
    m_languageCombo->setStyleSheet(R"(
        QComboBox {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFFFFF, stop:1 #F0F9FF);
            color: #3B82F6;
            border: 2px solid #BFDBFE;
            border-radius: 8px;
            padding: 6px 12px;
            font-size: 13px;
            font-weight: 600;
            min-width: 120px;
        }
        QComboBox:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #DBEAFE, stop:1 #BFDBFE);
            border-color: #60A5FA;
        }
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #3B82F6;
        }
    )");
    
    // 将图标和语言切换添加到菜单栏
    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    
    ui->menuBar->setCornerWidget(m_planeIconLabel, Qt::TopLeftCorner);
    
    QWidget* rightWidget = new QWidget(this);
    QHBoxLayout* rightLayout = new QHBoxLayout(rightWidget);
    rightLayout->setContentsMargins(10, 0, 10, 0);
    rightLayout->addWidget(m_languageCombo);
    ui->menuBar->setCornerWidget(rightWidget, Qt::TopRightCorner);
    
    // 连接语言切换信号
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, [this](int index) {
        Language lang = static_cast<Language>(m_languageCombo->itemData(index).toInt());
        LanguageManager::instance().switchLanguage(lang);
        updateUILanguage();
        
        // 显示切换成功提示
        QString title = lang == Language::Chinese ? 
            QString::fromUtf8("语言切换") : QString("Language Switch");
        QString msg = lang == Language::Chinese ? 
            QString::fromUtf8("已切换到中文") : QString("Switched to English");
        NotificationManager::instance().showNotification(title, msg, NotificationType::SystemMessage);
    });
}

// 初始化通知系统
void MainWindow::initNotificationSystem()
{
    NotificationManager::instance().initSystemTray(this);
    
    // 连接通知信号
    connect(&NotificationManager::instance(), &NotificationManager::notificationReceived,
            this, [](const NotificationItem& item) {
        qDebug() << "收到通知：" << item.title << "-" << item.message;
    });
    
    // 【新增】启动航班起飞提醒调度器
    FlightReminderScheduler::instance().startScheduler();
    qDebug() << "[MainWindow] 航班起飞提醒调度器已启动";
    
    // 显示欢迎通知（延迟显示，避免启动时冲突）
    QTimer::singleShot(1000, this, []() {
        NotificationManager::instance().showNotification(
            QString::fromUtf8("欢迎使用 FlyOver"),
            QString::fromUtf8("航班票务系统已启动，祝您旅途愉快！"),
            NotificationType::SystemMessage
        );
    });
}

// 更新UI语言
void MainWindow::updateUILanguage()
{
    Language currentLang = LanguageManager::instance().currentLanguage();
    
    // 更新窗口标题
    if (currentLang == Language::Chinese) {
        setWindowTitle(QString::fromUtf8("航班票务管理系统 - FlyOver"));
    } else {
        setWindowTitle(QString("Flight Booking System - FlyOver"));
    }
    
    // 更新侧边栏按钮文字
    ui->btnHome->setText(LanguageManager::instance().tr("home"));
    ui->btnFlightQuery->setText(LanguageManager::instance().tr("flight_query"));
    ui->btnOrders->setText(LanguageManager::instance().tr("my_orders"));
    ui->travelbutton->setText(LanguageManager::instance().tr("travel_moments"));
    ui->btnProfile->setText(LanguageManager::instance().tr("my_profile"));
    ui->btnAIService->setText(LanguageManager::instance().tr("ai_assistant"));
    
    // 管理员专属按钮（如果存在）
    if (m_isAdmin && ui->btnDataAnalytics) {
        ui->btnDataAnalytics->setText(LanguageManager::instance().tr("data_analytics"));
    }
    
    // 更新菜单栏
    if (ui->actionFlightManager) {
        ui->actionFlightManager->setText(LanguageManager::instance().tr("flight_management"));
    }
    
    // 更新其他UI元素...
    qDebug() << "UI语言已更新为：" << (currentLang == Language::Chinese ? "中文" : "English");
}

// 应用主题 - 强制浅色
void MainWindow::applyTheme(bool isDark)
{
    // 忽略参数，强制使用浅色主题
    Q_UNUSED(isDark);
    ThemeManager::instance().forceLightTheme();
}

// ==================== 主页面功能实现 ====================

// 初始化主页面
void MainWindow::initHomePage()
{
    // 创建会员信息卡片内容
    QFrame* memberCard = ui->memberInfoCard;
    
    // 检查是否已经有布局，避免重复创建
    QVBoxLayout* cardLayout = qobject_cast<QVBoxLayout*>(memberCard->layout());
    if (cardLayout) {
        // 如果已经有布局，清空所有子控件
        QLayoutItem* item;
        while ((item = cardLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                if (item->widget() == m_welcomeLabel) {
                    m_welcomeLabel = nullptr;
                }
                item->widget()->deleteLater();
            }
            delete item;
        }
    } else {
        // 第一次创建布局
        cardLayout = new QVBoxLayout(memberCard);
        cardLayout->setContentsMargins(30, 20, 30, 20);
    }
    
    // 欢迎标签
    if (m_welcomeLabel) {
        m_welcomeLabel->setParent(memberCard);
    } else {
        m_welcomeLabel = new QLabel(memberCard);
    }
    m_welcomeLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #1E40AF; font-family: 'Microsoft YaHei UI', 'SimHei'; background: transparent;");
    updateWelcomeLabel();
    cardLayout->addWidget(m_welcomeLabel);
    
    // 会员信息水平布局
    QHBoxLayout* infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(40);
    
    // 会员等级
    QVBoxLayout* levelLayout = new QVBoxLayout();
    QLabel* levelTitle = new QLabel(QString::fromUtf8("会员等级"), memberCard);
    levelTitle->setStyleSheet("font-size: 12px; color: #64748B; background: transparent;");
    m_memberLevelLabel = new QLabel(QString::fromUtf8("🥉 青铜会员"), memberCard);
    m_memberLevelLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E40AF; background: transparent;");
    levelLayout->addWidget(levelTitle);
    levelLayout->addWidget(m_memberLevelLabel);
    infoLayout->setSpacing(60);
    infoLayout->addLayout(levelLayout, 1);

    // 积分
    QVBoxLayout* pointsLayout = new QVBoxLayout();
    QLabel* pointsTitle = new QLabel(QString::fromUtf8("积分"), memberCard);
    pointsTitle->setStyleSheet("font-size: 12px; color: #64748B; background: transparent;");
    m_pointsLabel = new QLabel("0", memberCard);
    m_pointsLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E40AF; background: transparent;");
    pointsLayout->addWidget(pointsTitle);
    pointsLayout->addWidget(m_pointsLabel);
    infoLayout->addLayout(pointsLayout, 1);

    // 飞机币余额
    QVBoxLayout* balanceLayout = new QVBoxLayout();
    QLabel* balanceTitle = new QLabel(QString::fromUtf8("飞机币余额"), memberCard);
    balanceTitle->setStyleSheet("font-size: 12px; color: #64748B; background: transparent;");
    m_balanceLabel = new QLabel(QString::fromUtf8("¥10,000"), memberCard);
    m_balanceLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #10B981; background: transparent;");
    balanceLayout->addWidget(balanceTitle);
    balanceLayout->addWidget(m_balanceLabel);
    infoLayout->addLayout(balanceLayout, 1);

    // 飞行里程
    QVBoxLayout* mileageLayout = new QVBoxLayout();
    QLabel* mileageTitle = new QLabel(QString::fromUtf8("飞行里程"), memberCard);
    mileageTitle->setStyleSheet("font-size: 12px; color: #64748B; background: transparent;");
    m_mileageLabel = new QLabel("0 km", memberCard);
    m_mileageLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #1E40AF; background: transparent;");
    mileageLayout->addWidget(mileageTitle);
    mileageLayout->addWidget(m_mileageLabel);
    infoLayout->addLayout(mileageLayout, 1);
    cardLayout->addLayout(infoLayout);
    
    // 创建快捷功能按钮
    createQuickActionButtons();
    
    // 注释掉统计卡片（用户要求去掉"我的统计"）
    // createStatisticsCards();
    
    // 更新会员信息
    updateMemberInfo();

    // 确保主页不显示滚动条，内容水平居中，并禁用滚动区的键盘焦点（避免方向键移动）
    if (ui->homeScrollArea) {
        ui->homeScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->homeScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->homeScrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        // 允许内部自适应缩放，避免固定宽度被裁切
        ui->homeScrollArea->setWidgetResizable(true);

        // 禁用滚动区域及其 viewport 的键盘焦点，阻止箭头键滚动
        ui->homeScrollArea->setFocusPolicy(Qt::NoFocus);
        if (ui->homeScrollArea->viewport()) ui->homeScrollArea->viewport()->setFocusPolicy(Qt::NoFocus);

        // 将内部内容容器宽度限制为合适值并水平居中布局
        QWidget *inner = ui->homeScrollArea->widget();
        if (inner) {
            inner->setContentsMargins(0, 0, 0, 0);
            int sideBarW = (ui->sideBar ? ui->sideBar->width() : 180);
            int viewportW = ui->homeScrollArea->viewport() ? ui->homeScrollArea->viewport()->width() : 0;
            int baseW = viewportW > 0 ? viewportW : (this->width() - sideBarW - 40);
            int minW = 680;
            int maxW = 1600; // 放宽上限，避免宽屏时内容被截断
            // int targetW = std::clamp(baseW - 40, minW, maxW); // Unused
            inner->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            inner->setMinimumWidth(minW);
            inner->setMaximumWidth(maxW);

            // 会员信息卡允许伸缩但给出推荐最大宽度
            if (memberCard) {
                memberCard->setMinimumWidth(640);
                memberCard->setMaximumWidth(maxW - 60);
                memberCard->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            }

            // 将内部 layout 水平对齐到中间
            QLayout *lyt = inner->layout();
            if (lyt) {
                lyt->setContentsMargins(0, 0, 0, 0);
                lyt->setSpacing(24);
                // 针对 QBoxLayout 设置对齐
                if (auto box = qobject_cast<QBoxLayout*>(lyt)) {
                    box->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
                }
            }
        }
    }
}

// 创建快捷功能按钮
void MainWindow::createQuickActionButtons()
{
    QFrame* actionsFrame = ui->quickActionsFrame;
    
    // 移除旧布局并重建为等宽网格布局，确保按钮均匀分布
    QLayout *old = actionsFrame->layout();
    if (old) {
        QLayoutItem* it;
        while ((it = old->takeAt(0)) != nullptr) {
            if (it->widget()) it->widget()->deleteLater();
            delete it;
        }
        delete old;
    }

    QGridLayout* grid = new QGridLayout(actionsFrame);
    grid->setContentsMargins(20, 8, 20, 8);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setAlignment(Qt::AlignHCenter);

    // 快捷按钮样式
    QString buttonStyle = R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F8FAFC);
            border: 2px solid #E2E8F0;
            border-radius: 16px;
            padding: 20px;
            font-size: 15px;
            font-weight: 600;
            color: #475569;
            font-family: 'Microsoft YaHei UI', 'SimHei';
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #DBEAFE, stop:1 #BFDBFE);
            border-color: #60A5FA;
            color: #1E40AF;
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #BFDBFE, stop:1 #93C5FD);
        }
    )";
    
    // 航班查询按钮
    QPushButton* btnQuickFlight = new QPushButton(QString::fromUtf8("✈️\n航班查询"), actionsFrame);
    btnQuickFlight->setMinimumSize(130, 94);
    btnQuickFlight->setMaximumSize(150, 104);
    btnQuickFlight->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnQuickFlight->setStyleSheet(buttonStyle);
    connect(btnQuickFlight, &QPushButton::clicked, this, &MainWindow::on_btnFlightQuery_clicked);
    QWidget *wrapFlight = new QWidget(actionsFrame);
    QHBoxLayout *wrapL1 = new QHBoxLayout(wrapFlight);
    wrapL1->setContentsMargins(4,6,4,6);
    wrapL1->addWidget(btnQuickFlight, 0, Qt::AlignCenter);
    grid->addWidget(wrapFlight, 0, 0);

    // 我的订单按钮
    QPushButton* btnQuickOrders = new QPushButton(QString::fromUtf8("🧾\n我的订单"), actionsFrame);
    btnQuickOrders->setMinimumSize(130, 94);
    btnQuickOrders->setMaximumSize(150, 104);
    btnQuickOrders->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnQuickOrders->setStyleSheet(buttonStyle);
    connect(btnQuickOrders, &QPushButton::clicked, this, &MainWindow::on_btnOrders_clicked);
    QWidget *wrapOrders = new QWidget(actionsFrame);
    QHBoxLayout *wrapL2 = new QHBoxLayout(wrapOrders);
    wrapL2->setContentsMargins(4,6,4,6);
    wrapL2->addWidget(btnQuickOrders, 0, Qt::AlignCenter);
    grid->addWidget(wrapOrders, 0, 1);

    // 地图可视化按钮
    QPushButton* btnQuickMap = new QPushButton(QString::fromUtf8("🗺️\n航线地图"), actionsFrame);
    btnQuickMap->setMinimumSize(130, 94);
    btnQuickMap->setMaximumSize(150, 104);
    btnQuickMap->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnQuickMap->setStyleSheet(buttonStyle);
    connect(btnQuickMap, &QPushButton::clicked, this, []() {
        // 创建地图可视化窗口
        MapVisualization* mapWindow = new MapVisualization();
        mapWindow->setWindowTitle(QString::fromUtf8("航线地图 - 中国城市分布"));
        mapWindow->resize(1000, 700);
        
        // 从数据库加载所有航班数据
        QList<Flight> flights = DBManager::instance().getAllFlights();
        mapWindow->setFlightData(flights);
        
        mapWindow->show();
    });
    QWidget *wrapMap = new QWidget(actionsFrame);
    QHBoxLayout *wrapL3 = new QHBoxLayout(wrapMap);
    wrapL3->setContentsMargins(4,6,4,6);
    wrapL3->addWidget(btnQuickMap, 0, Qt::AlignCenter);
    grid->addWidget(wrapMap, 0, 2);

    // AI客服按钮
    QPushButton* btnQuickAI = new QPushButton(QString::fromUtf8("💬\nAI 客服"), actionsFrame);
    btnQuickAI->setMinimumSize(130, 94);
    btnQuickAI->setMaximumSize(150, 104);
    btnQuickAI->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnQuickAI->setStyleSheet(buttonStyle);
    connect(btnQuickAI, &QPushButton::clicked, this, &MainWindow::on_btnAIService_clicked);
    QWidget *wrapAI = new QWidget(actionsFrame);
    QHBoxLayout *wrapL4 = new QHBoxLayout(wrapAI);
    wrapL4->setContentsMargins(4,6,4,6);
    wrapL4->addWidget(btnQuickAI, 0, Qt::AlignCenter);
    grid->addWidget(wrapAI, 0, 3);

    // 积分商城按钮
    QPushButton* btnPointsShop = new QPushButton(QString::fromUtf8("🎁\n积分商城"), actionsFrame);
    btnPointsShop->setMinimumSize(130, 94);
    btnPointsShop->setMaximumSize(150, 104);
    btnPointsShop->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btnPointsShop->setStyleSheet(buttonStyle);
    connect(btnPointsShop, &QPushButton::clicked, this, [this]() {
        PointsShopDialog* dlg = new PointsShopDialog(m_appUser.account, this);
        dlg->exec();
        dlg->deleteLater();
        // 兑换后更新会员信息显示
        updateMemberInfo();
    });
    QWidget *wrapPoints = new QWidget(actionsFrame);
    QHBoxLayout *wrapL5 = new QHBoxLayout(wrapPoints);
    wrapL5->setContentsMargins(4,6,4,6);
    wrapL5->addWidget(btnPointsShop, 0, Qt::AlignCenter);
    grid->addWidget(wrapPoints, 0, 4);

    // 单行五列，等比拉伸列，水平排列完整显示
    for (int c = 0; c < 5; ++c) grid->setColumnStretch(c, 1);
    grid->setRowStretch(0, 1);
}

// 创建统计卡片
void MainWindow::createStatisticsCards()
{
    QFrame* statsFrame = ui->statisticsFrame;
    QHBoxLayout* layout = new QHBoxLayout(statsFrame);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 10, 40, 10);
    
    // 统计卡片样式
    QString cardStyle = R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #FFFFFF, stop:1 #F8FAFC);
            border: 2px solid #E2E8F0;
            border-radius: 12px;
            padding: 15px;
        }
    )";
    
    // 本月订单数
    QFrame* ordersCard = new QFrame(statsFrame);
    ordersCard->setMinimumSize(180, 80);
    ordersCard->setStyleSheet(cardStyle);
    QVBoxLayout* ordersLayout = new QVBoxLayout(ordersCard);
    QLabel* ordersIcon = new QLabel(QString::fromUtf8("📋"), ordersCard);
    ordersIcon->setStyleSheet("font-size: 24px;");
    QLabel* ordersTitle = new QLabel(QString::fromUtf8("本月订单"), ordersCard);
    ordersTitle->setStyleSheet("font-size: 12px; color: #64748B;");
    QLabel* ordersValue = new QLabel("0", ordersCard);
    ordersValue->setStyleSheet("font-size: 20px; font-weight: bold; color: #3B82F6;");
    ordersLayout->addWidget(ordersIcon);
    ordersLayout->addWidget(ordersTitle);
    ordersLayout->addWidget(ordersValue);
    layout->addWidget(ordersCard);
    
    // 本月消费
    QFrame* spendingCard = new QFrame(statsFrame);
    spendingCard->setMinimumSize(180, 80);
    spendingCard->setStyleSheet(cardStyle);
    QVBoxLayout* spendingLayout = new QVBoxLayout(spendingCard);
    QLabel* spendingIcon = new QLabel(QString::fromUtf8("💰"), spendingCard);
    spendingIcon->setStyleSheet("font-size: 24px;");
    QLabel* spendingTitle = new QLabel(QString::fromUtf8("本月消费"), spendingCard);
    spendingTitle->setStyleSheet("font-size: 12px; color: #64748B;");
    QLabel* spendingValue = new QLabel(QString::fromUtf8("¥0"), spendingCard);
    spendingValue->setStyleSheet("font-size: 20px; font-weight: bold; color: #10B981;");
    spendingLayout->addWidget(spendingIcon);
    spendingLayout->addWidget(spendingTitle);
    spendingLayout->addWidget(spendingValue);
    layout->addWidget(spendingCard);
    
    // 累计飞行城市
    QFrame* citiesCard = new QFrame(statsFrame);
    citiesCard->setMinimumSize(180, 80);
    citiesCard->setStyleSheet(cardStyle);
    QVBoxLayout* citiesLayout = new QVBoxLayout(citiesCard);
    QLabel* citiesIcon = new QLabel(QString::fromUtf8("🌆"), citiesCard);
    citiesIcon->setStyleSheet("font-size: 24px;");
    QLabel* citiesTitle = new QLabel(QString::fromUtf8("飞行城市"), citiesCard);
    citiesTitle->setStyleSheet("font-size: 12px; color: #64748B;");
    QLabel* citiesValue = new QLabel("0", citiesCard);
    citiesValue->setStyleSheet("font-size: 20px; font-weight: bold; color: #8B5CF6;");
    citiesLayout->addWidget(citiesIcon);
    citiesLayout->addWidget(citiesTitle);
    citiesLayout->addWidget(citiesValue);
    layout->addWidget(citiesCard);
    
    layout->addStretch();
}

void MainWindow::updateWelcomeLabel()
{
    if (!m_welcomeLabel) {
        return;
    }

    QString displayName = m_appUser.nickname.trimmed();
    if (displayName.isEmpty()) {
        displayName = QString::fromUtf8("用户");
    }

    m_welcomeLabel->setText(QString::fromUtf8("你好，") + displayName);
}

// 更新会员信息
void MainWindow::updateMemberInfo()
{
    updateWelcomeLabel();

    // 安全检查：确保有有效的用户账号
    if (m_appUser.account.isEmpty()) {
        qDebug() << "[会员信息] 用户账号为空，跳过更新";
        return;
    }
    
    // 从会员系统获取当前用户的会员信息
    MemberInfo memberInfo = MemberSystem::instance().getMemberInfo(m_appUser.account);
    
    // 更新等级
    if (m_memberLevelLabel) {
        m_memberLevelLabel->setText(memberInfo.getLevelIcon() + " " + memberInfo.getLevelName());
    }
    
    // 更新积分
    if (m_pointsLabel) {
        m_pointsLabel->setText(QString::number(memberInfo.points));
    }
    
    // 更新余额
    if (m_balanceLabel) {
        m_balanceLabel->setText(QString::fromUtf8("¥") + QString::number(memberInfo.balance, 'f', 2));
    }
    
    // 更新里程
    if (m_mileageLabel) {
        m_mileageLabel->setText(QString::number(memberInfo.mileage, 'f', 0) + " km");
    }
}

// ==================== 主页面功能实现结束 ====================

