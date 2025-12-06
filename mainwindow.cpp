#include "mainwindow.h"
#include "login.h"
#include "ui_mainwindow.h"
#include "views/travelmoment.h"
#include <QPropertyAnimation>
#include <QEasingCurve>
#include "ai.h"
#include<ordermanager.h>
#include "ProfileDisplayDialog.h"
#include <QMouseEvent>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_flightManager(nullptr),
    m_aiquery(nullptr),
    m_isAdmin(false),
    m_orderManager(nullptr)
{
    ui->setupUi(this);

    // 原有信号槽（保留）
    connect(ui->btnHome, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->pageHome);
    });
    connect(ui->travelbutton, &QPushButton::clicked, this, &MainWindow::onTravelButtonClicked);
    connect(ui->btnFlightQuery, &QPushButton::clicked, this, &MainWindow::on_btnFlightQuery_clicked);
    connect(ui->btnOrders, &QPushButton::clicked, this, &MainWindow::on_btnOrders_clicked);
    connect(ui->actionViewOrders, &QAction::triggered, this, &MainWindow::on_btnOrders_clicked);
    connect(ui->btnProfile,&QPushButton::clicked,this,&MainWindow::clicked_btnProfile);
    // 【新增】提前初始化OrderManager（确保信号连接时已存在）
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
    }

    setWindowTitle("航班票务系统 - 主菜单");
    setIsAdmin(true);
    // 注意：account 会在登录成功后通过 setUserProfile 被设置
    m_appUser.account = "";
    m_appUser.nickname = "用户";
    m_appUser.gender = Gender::Unknown;
    m_appUser.phone = "";
    m_appUser.email = "";
    m_appUser.avatar = QPixmap(":/images/default_avatar.jpg");

    // 创建浮动小球 AI 按钮（初始在主窗口左侧隐藏）
    m_floatingAIButton = new QPushButton(this);
    m_floatingAIButton->setObjectName("floatingAIButton");
    m_floatingAIButton->setFixedSize(40, 40); // 小球尺寸
    m_floatingAIButton->setToolTip("AI Assistant");
    m_floatingAIButton->setFlat(true);
    m_floatingAIButton->setStyleSheet("background: transparent; border: none;");
    // 简化：绘制蓝色圆，中心绘制黑色大写 'T'
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
    // 仅绘制纯色圆（不绘制文字）
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
    // 使用带动画的显示方法
    aiq->showWithAnimation();
}
void MainWindow::set_account(QString acc){
    m_appUser.account = acc;
}

void MainWindow::setUserProfile(const UserProfile& profile) {
    m_appUser = profile;
    qDebug() << "MainWindow::setUserProfile 被调用"
             << "account=" << profile.account
             << "nickname=" << profile.nickname
             << "phone=" << profile.phone
             << "email=" << profile.email;
}
MainWindow::~MainWindow()
{
    delete ui;
    // 无需手动删除m_flightManager（父对象为stackedWidget，会自动释放）
}

// 实现权限设置函数
void MainWindow::setIsAdmin(bool isAdmin)
{
    m_isAdmin = isAdmin;
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

        // 【新增】连接信号（管理员模式购票也需要同步订单）
        connect(m_flightManager, &FlightManager::orderCreated,
                m_orderManager, &OrderManager::refreshOrderList);
    }
    m_flightManager->setAdminMode(true); // 管理员模式
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}
void MainWindow::on_btnFlightQuery_clicked()
{
    if (!m_flightManager) {
        m_flightManager = new FlightManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_flightManager);

        // 【新增】连接FlightManager的订单创建信号到OrderManager的刷新方法
        connect(m_flightManager, &FlightManager::orderCreated,
                m_orderManager, &OrderManager::refreshOrderList);
    }
    m_flightManager->setAdminMode(false); // 普通查询模式
    ui->stackedWidget->setCurrentWidget(m_flightManager);
}
void MainWindow::on_btnOrderManager_clicked() // 假设主窗口有“订单管理”按钮
{
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
    }
    ui->stackedWidget->setCurrentWidget(m_orderManager);
}
void MainWindow::on_btnOrders_clicked()
{
    if (!m_orderManager) {
        m_orderManager = new OrderManager(ui->stackedWidget);
        ui->stackedWidget->addWidget(m_orderManager);
    }
    ui->stackedWidget->setCurrentWidget(m_orderManager);
}

// 新增：退出登录逻辑
void MainWindow::on_actionLogout_triggered()
{
    // 只弹出一次确认框（因已修复重复连接问题）
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "退出登录", "确定要退出当前账号吗？",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
    if (reply != QMessageBox::Yes) {
        return;
    }

    // 创建登录窗口（无父对象，避免被主窗口关闭影响）
    LoginDialog *loginDialog = new LoginDialog();
    loginDialog->setAttribute(Qt::WA_DeleteOnClose);
    loginDialog->show(); // 先显示登录窗口

    this->close(); // 再关闭主窗口（此时登录窗口已存在，程序不退出）
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

    // 组合信息文本，格式清晰
    QString aboutText = QString("%1\n\n"
                                "版本：%2\n"
                                "发布日期：%3\n\n"
                                "%4\n\n"
                                "%5\n\n"
                                "%6")
                            .arg(systemName)
                            .arg(version)
                            .arg(releaseDate)
                            .arg(description)
                            .arg(copyright)
                            .arg(contact);

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
            QPoint global = me->globalPos();
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
