#include "login.h"
#include "ui_login.h"
#include "dbmanager.h"
#include "mainwindow.h"
#include "forgotpassworddialog.h"
#include "utils.h"
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QInputDialog>
#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include <QRandomGenerator>
#include <QDebug>
#include <QHideEvent>
#include <QShowEvent>
#include <cmath>

// ========== ParticlePanel 实现 (==========
ParticlePanel::ParticlePanel(QWidget *parent)
    : QWidget(parent), m_gradientOffset(0.0f), m_timerId(0)
{
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAttribute(Qt::WA_OpaquePaintEvent); // 性能优化：声明不透明绘制
    initParticles();
    startAnimation();
}

ParticlePanel::~ParticlePanel()
{
    stopAnimation();
}

void ParticlePanel::startAnimation()
{
    if (m_timerId == 0) {
        m_timerId = startTimer(50); // ~20 FPS
        qDebug() << "[ParticlePanel] 动画已启动, timerId=" << m_timerId;
    } else {
        qDebug() << "[ParticlePanel] 动画已在运行, timerId=" << m_timerId;
    }
}

void ParticlePanel::stopAnimation()
{
    if (m_timerId != 0) {
        killTimer(m_timerId);
        qDebug() << "[ParticlePanel] 动画已停止, timerId=" << m_timerId;
        m_timerId = 0;
    }
}

void ParticlePanel::initParticles()
{
    m_particles.clear();
    QRandomGenerator *rng = QRandomGenerator::global();
    
    // 减少粒子数量到15个 (性能优化)
    for (int i = 0; i < 15; i++) {
        Particle p;
        p.pos = QPointF(rng->bounded(380), rng->bounded(520));
        p.velocity = QPointF(
            (rng->bounded(60) - 30) / 100.0f,
            -0.2f - rng->bounded(30) / 100.0f // 缓慢向上飘动
        );
        p.alpha = rng->bounded(20, 50) / 100.0f;
        p.size = rng->bounded(4, 12);
        p.alphaSpeed = (rng->bounded(40) - 20) / 1000.0f;
        m_particles.append(p);
    }
}

void ParticlePanel::updateParticles()
{
    QRandomGenerator *rng = QRandomGenerator::global();
    
    for (int i = 0; i < m_particles.size(); i++) {
        Particle &p = m_particles[i];
        
        // 更新位置
        p.pos += p.velocity;
        
        // 更新透明度（呼吸效果）
        p.alpha += p.alphaSpeed;
        if (p.alpha > 0.6f) p.alphaSpeed = -std::abs(p.alphaSpeed);
        if (p.alpha < 0.15f) p.alphaSpeed = std::abs(p.alphaSpeed);
        
        // 边界检查，循环出现
        if (p.pos.y() < -20) {
            p.pos.setY(height() + 10);
            p.pos.setX(rng->bounded(width()));
        }
        if (p.pos.x() < -20) p.pos.setX(width() + 10);
        if (p.pos.x() > width() + 20) p.pos.setX(-10);
    }
    
    // 缓慢的渐变动画偏移 (性能优化)
    m_gradientOffset += 0.001f;
    if (m_gradientOffset > 1.0f) m_gradientOffset -= 1.0f;
}

void ParticlePanel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId) {
        updateParticles();
        update(); // 触发重绘
    }
}

void ParticlePanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 高级蓝色渐变背景 (深海蓝 #0A3C5F → 天空青 #3498DB)
    QLinearGradient gradient(0, 0, width(), height());
    
    // 使用偏移创建缓慢动态效果
    float offset = m_gradientOffset;
    gradient.setColorAt(0.0, QColor(10, 60, 95));     // 深海蓝 #0A3C5F
    gradient.setColorAt(0.4 + offset * 0.1, QColor(26, 82, 118));  // 过渡蓝
    gradient.setColorAt(0.7 + offset * 0.1, QColor(52, 152, 219)); // 天空青 #3498DB
    gradient.setColorAt(1.0, QColor(41, 128, 185));   // 中蓝色
    
    painter.fillRect(rect(), gradient);
    
    // 绘制轻量级粒子（圆形光点）
    for (const Particle &p : m_particles) {
        QColor particleColor(255, 255, 255, int(p.alpha * 180));
        painter.setBrush(particleColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(p.pos, p.size, p.size);
    }
}

// ========== LoginDialog 实现 ==========

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog),
    registerDialog(nullptr),
    m_particlePanel(nullptr)
{
    qDebug() << "[LoginDialog] 构造函数被调用";
    
    ui->setupUi(this);
    setWindowTitle("登录 - 航班票务系统");
    setFixedSize(850, 520);
    
    // 隐藏 UI 文件中的默认布局内容
    if (ui->titleLabel) ui->titleLabel->hide();
    if (ui->accountLabel) ui->accountLabel->hide();
    if (ui->passwordLabel) ui->passwordLabel->hide();
    
    // 清除 UI 文件中的默认布局，创建新的自定义布局
    if (layout()) {
        QLayoutItem *item;
        while ((item = layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->hide();
            }
            delete item;
        }
        delete layout();
    }
    
    // 创建主水平布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // ========== 左侧粒子动画区域 (蓝紫渐变 + 粒子) ==========
    m_particlePanel = new ParticlePanel(this);
    m_particlePanel->setFixedWidth(380);
    m_particlePanel->setMinimumHeight(520);
    
    QVBoxLayout *leftLayout = new QVBoxLayout(m_particlePanel);
    leftLayout->setContentsMargins(40, 60, 40, 60);
    leftLayout->setSpacing(20);
    
    // 飞机图标
    QLabel *iconLabel = new QLabel("✈", m_particlePanel);
    iconLabel->setStyleSheet("font-size: 72px; color: white; background: transparent;");
    iconLabel->setAlignment(Qt::AlignCenter);
    
    // 系统名称
    QLabel *brandLabel = new QLabel("FlyOver", m_particlePanel);
    brandLabel->setStyleSheet(R"(
        font-size: 42px; 
        font-weight: bold; 
        color: #FFFFFF; 
        background: transparent; 
        letter-spacing: 2px;
    )");
    brandLabel->setAlignment(Qt::AlignCenter);
    // 添加强化阴影效果确保WCAG AA标准对比度
    QGraphicsDropShadowEffect *brandShadow = new QGraphicsDropShadowEffect;
    brandShadow->setBlurRadius(15);
    brandShadow->setColor(QColor(0, 0, 0, 200));
    brandShadow->setOffset(0, 2);
    brandLabel->setGraphicsEffect(brandShadow);
    
    // Slogan
    QLabel *sloganLabel = new QLabel("探索世界，从这里起飞", m_particlePanel);
    sloganLabel->setStyleSheet(R"(
        font-size: 18px; 
        color: #FFFFFF; 
        background: transparent;
        font-weight: 500;
    )");
    sloganLabel->setAlignment(Qt::AlignCenter);
    // 添加强化阴影效果确保WCAG AA标准对比度
    QGraphicsDropShadowEffect *sloganShadow = new QGraphicsDropShadowEffect;
    sloganShadow->setBlurRadius(12);
    sloganShadow->setColor(QColor(0, 0, 0, 200));
    sloganShadow->setOffset(0, 2);
    sloganLabel->setGraphicsEffect(sloganShadow);
    
    // 功能亮点
    QLabel *featuresLabel = new QLabel(
        "🎫 便捷购票 · 智能推荐\n"
        "📊 数据分析 · 一目了然\n"
        "💡 AI 助手 · 贴心服务",
        m_particlePanel
    );
    featuresLabel->setStyleSheet(R"(
        font-size: 14px; 
        color: #FFFFFF; 
        background: transparent; 
        line-height: 180%;
    )");
    featuresLabel->setAlignment(Qt::AlignCenter);
    // 添加强化阴影效果确保WCAG AA标准对比度
    QGraphicsDropShadowEffect *featuresShadow = new QGraphicsDropShadowEffect;
    featuresShadow->setBlurRadius(10);
    featuresShadow->setColor(QColor(0, 0, 0, 200));
    featuresShadow->setOffset(0, 1);
    featuresLabel->setGraphicsEffect(featuresShadow);
    
    leftLayout->addStretch(1);
    leftLayout->addWidget(iconLabel);
    leftLayout->addWidget(brandLabel);
    leftLayout->addWidget(sloganLabel);
    leftLayout->addSpacing(30);
    leftLayout->addWidget(featuresLabel);
    leftLayout->addStretch(2);
    
    // ========== 右侧表单区域 ==========
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setStyleSheet(R"(
        QWidget {
            background-color: #ffffff;
            border-top-right-radius: 12px;
            border-bottom-right-radius: 12px;
        }
    )");
    
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(50, 50, 50, 40);
    rightLayout->setSpacing(16);
    
    // 欢迎标题
    QLabel *welcomeLabel = new QLabel("欢迎使用！", rightPanel);
    welcomeLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #333; background: transparent;");
    
    QLabel *subtitleLabel = new QLabel("请登录您的账号", rightPanel);
    subtitleLabel->setStyleSheet(R"(
        font-size: 15px; 
        color: #666; 
        background: transparent; 
        margin-bottom: 20px;
        padding: 4px 0px;
        height: 80px;
    )");
    subtitleLabel->setMinimumHeight(48);
    
    // 账号输入
    QLabel *accountLabel = new QLabel("账号", rightPanel);
    accountLabel->setStyleSheet("font-size: 13px; color: #555; font-weight: 500; background: transparent;");
    
    // 使用原有的accountEdit
    ui->accountEdit->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 14px;
            background-color: #fafafa;
            color: #333;

        }
        QLineEdit:focus {
            border-color: #3498DB;
            background-color: #fff;
        }
    )");
    ui->accountEdit->setPlaceholderText("请输入账号");
    ui->accountEdit->setMinimumHeight(48);
    
    // 密码输入
    QLabel *passwordLabel = new QLabel("密码", rightPanel);
    passwordLabel->setStyleSheet("font-size: 13px; color: #555; font-weight: 500; background: transparent;");
    
    ui->passwordEdit->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px 16px;
            font-size: 14px;
            background-color: #fafafa;
            color: #333;
        }
        QLineEdit:focus {
            border-color: #3498DB;
            background-color: #fff;
        }
    )");
    ui->passwordEdit->setPlaceholderText("请输入密码");
    ui->passwordEdit->setMinimumHeight(48);
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    
    // 记住我 + 忘记密码行
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    
    m_rememberCheck = new QCheckBox("记住我", rightPanel);
    m_rememberCheck->setStyleSheet(R"(
        QCheckBox {
            font-size: 13px;
            color: #666;
            background: transparent;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
        }
    )");
    
    ui->forgotPasswordLink->setStyleSheet(R"(
        QPushButton {
            font-size: 13px;
            color: #3498DB;
            background: transparent;
            border: none;
            text-decoration: none;
        }
        QPushButton:hover {
            color: #0A3C5F;
            text-decoration: underline;
        }
    )");
    ui->forgotPasswordLink->setText("忘记密码？");
    ui->forgotPasswordLink->setCursor(Qt::PointingHandCursor);
    
    optionsLayout->addWidget(m_rememberCheck);
    optionsLayout->addStretch();
    optionsLayout->addWidget(ui->forgotPasswordLink);
    
    // 管理员入口链接（紧凑样式）
    QPushButton *adminToggleBtn = new QPushButton("🔑 管理员登录", rightPanel);
    adminToggleBtn->setStyleSheet(R"(
        QPushButton {
            font-size: 12px;
            color: #F39C12;
            background: transparent;
            border: none;
            text-align: center;
            padding: 2px;
        }
        QPushButton:hover {
            color: #E67E22;
            text-decoration: underline;
        }
    )");
    adminToggleBtn->setCursor(Qt::PointingHandCursor);
    connect(adminToggleBtn, &QPushButton::clicked, this, &LoginDialog::toggleAdminCodeInput);
    
    // 登录按钮 - 深海蓝渐变
    ui->loginButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0A3C5F, stop:1 #3498DB);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 14px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1A5276, stop:1 #5DADE2);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #0A3C5F, stop:1 #2980B9);
        }
    )");
    ui->loginButton->setText("登 录");
    ui->loginButton->setMinimumHeight(50);
    ui->loginButton->setCursor(Qt::PointingHandCursor);
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *loginShadow = new QGraphicsDropShadowEffect();
    loginShadow->setBlurRadius(20);
    loginShadow->setColor(QColor(10, 60, 95, 80));
    loginShadow->setOffset(0, 4);
    ui->loginButton->setGraphicsEffect(loginShadow);
    
    // 注册链接
    QHBoxLayout *registerLayout = new QHBoxLayout();
    QLabel *noAccountLabel = new QLabel("还没有账号？", rightPanel);
    noAccountLabel->setStyleSheet("font-size: 13px; color: #888; background: transparent;");
    
    ui->registerLink->setStyleSheet(R"(
        QPushButton {
            font-size: 13px;
            color: #3498DB;
            background: transparent;
            border: none;
            font-weight: bold;
        }
        QPushButton:hover {
            color: #0A3C5F;
            text-decoration: underline;
        }
    )");
    ui->registerLink->setText("立即注册");
    ui->registerLink->setCursor(Qt::PointingHandCursor);
    
    registerLayout->addStretch();
    registerLayout->addWidget(noAccountLabel);
    registerLayout->addWidget(ui->registerLink);
    registerLayout->addStretch();
    
    // 组装右侧布局
    rightLayout->addWidget(welcomeLabel);
    rightLayout->addWidget(subtitleLabel);
    rightLayout->addSpacing(10);
    rightLayout->addWidget(accountLabel);
    rightLayout->addWidget(ui->accountEdit);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(passwordLabel);
    rightLayout->addWidget(ui->passwordEdit);
    rightLayout->addSpacing(4);
    rightLayout->addLayout(optionsLayout);
    rightLayout->addSpacing(2);
    rightLayout->addWidget(adminToggleBtn, 0, Qt::AlignCenter);  // 管理员入口按钮（居中）
    rightLayout->addSpacing(16);
    rightLayout->addWidget(ui->loginButton);
    rightLayout->addSpacing(20);
    rightLayout->addLayout(registerLayout);
    rightLayout->addStretch();
    
    // 组装主布局 - 使用粒子动画面板
    mainLayout->addWidget(m_particlePanel);
    mainLayout->addWidget(rightPanel, 1);
    
    // 连接忘记密码链接信号
    connect(ui->forgotPasswordLink, &QPushButton::clicked, this, &LoginDialog::onForgotPasswordClicked);
    
    // 加载保存的凭证 (记住我功能)
    loadRememberedCredentials();
}

LoginDialog::~LoginDialog()
{
    qDebug() << "[LoginDialog] 析构函数被调用";
    delete ui;
}


// 窗口隐藏时停止动画
void LoginDialog::hideEvent(QHideEvent *event)
{
    qDebug() << "[LoginDialog] hideEvent - 停止粒子动画";
    if (m_particlePanel) {
        m_particlePanel->stopAnimation();
    }
    QDialog::hideEvent(event);
}

// 窗口显示时启动动画
void LoginDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    qDebug() << "[LoginDialog] showEvent - 启动粒子动画";
    if (m_particlePanel) {
        m_particlePanel->startAnimation();
    }
}

void LoginDialog::on_loginButton_clicked()
{
    QString account = ui->accountEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    // 输入验证
    if (account.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "账号和密码不能为空！");
        return;
    }

    // 新增：账号格式校验（与注册逻辑一致）
    if (account.length() < 4 || account.length() > 20 ||
        !account.contains(QRegularExpression("^[A-Za-z0-9]+$"))) {
        QMessageBox::warning(this, "提示", "账号需为4-20位字母或数字！");
        return;
    }
    // 新增：登录锁定检查
    if (QDateTime::currentDateTime() < m_lockUntil) {
        int left = QDateTime::currentDateTime().secsTo(m_lockUntil);
        QMessageBox::warning(this, "锁定中", QString("连续输错5次，还需等待%1秒").arg(left));
        return;
    }

    // 直接调用DBManager获取数据库连接
    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!db.isOpen()) {
        return; // DBManager已弹出连接失败提示，无需重复处理
    }

    // 验证账号密码
    QSqlQuery query(db);
    query.prepare("SELECT password, salt FROM users WHERE account = :account"); // 注意表名是users（DBManager中创建的是users）
    query.bindValue(":account", account);

    if (!query.exec()) {
        QMessageBox::critical(this, "错误", "查询失败：" + query.lastError().text());
        return;
    }

    if (query.next()) {
        // 新增：加密比对
        QString storedPwd = query.value(0).toString();
        QString salt = query.value(1).toString();

        // 处理盐值为空的情况（如旧用户）
        if (salt.isEmpty()) {
            QMessageBox::warning(this, "错误", "账号信息不完整，请联系管理员重置密码！");
            return;
        }

        QString inputPwd = hashPassword(password, salt);

        if (inputPwd == storedPwd) {
            // 登录成功 - 检查管理员验证码
            bool isAdmin = false;
            
            // 检查是否已输入正确的管理员验证码
            if (!m_adminCode.isEmpty() && m_adminCode == "Admin") {
                isAdmin = true;
                qDebug() << "管理员验证码正确，授予管理员权限";
            }
            
            QMessageBox::information(this, "成功", 
                isAdmin ? "登录成功！（管理员模式）" : "登录成功！");
            m_failedAttempts = 0; // 重置失败次数
            
            // 记住我功能：保存或清除凭证
            if (m_rememberCheck && m_rememberCheck->isChecked()) {
                saveCredentials();
            } else {
                clearCredentials();
            }

            // 从数据库加载用户信息
            UserProfile userProfile = DBManager::instance().loadUserProfile(account);
            
            // 使用验证码结果覆盖数据库角色
            if (isAdmin) {
                userProfile.role = UserRole::Admin;
            } else {
                userProfile.role = UserRole::User;
            }
            
            qDebug() << "登录成功，加载的账号:" << userProfile.account 
                     << "角色:" << (userProfile.isAdmin() ? "管理员" : "普通用户");
            
            // 创建主窗口
            MainWindow *mainWin = new MainWindow(nullptr);
            mainWin->setAttribute(Qt::WA_DeleteOnClose);
            
            // 将加载的用户信息传递给主窗口
            mainWin->setUserProfile(userProfile);
            
            // 根据角色设置管理员权限
            mainWin->setIsAdmin(userProfile.isAdmin());
            
            mainWin->show();
            
            qDebug() << "[LoginDialog] 主窗口已显示，隐藏登录窗口";
            
            // 隐藏登录窗口（不删除，以便退出登录时重用）
            this->hide();
        } else {
            // 密码错误（新增失败次数限制）
            m_failedAttempts++;
            int remaining = 5 - m_failedAttempts;
            if (remaining <= 0) {
                m_lockUntil = QDateTime::currentDateTime().addSecs(60);
                QMessageBox::warning(this, "失败", "密码错误，账号已锁定1分钟！");
            } else {
                QMessageBox::warning(this, "失败", QString("密码错误，还可尝试%1次").arg(remaining));
            }
            ui->passwordEdit->clear();
        }
    } else {
        QMessageBox::warning(this, "失败", "账号不存在！");
        ui->accountEdit->clear();
        ui->passwordEdit->clear();
    }
}
QString LoginDialog::get_account()const{
    return ui->accountEdit->text().trimmed();
}
void LoginDialog::on_registerLink_clicked()
{
    if (!registerDialog) {
        registerDialog.reset(new RegisterDialog(this));
        registerDialog->setWindowTitle("注册");
    }
    registerDialog->exec();
}

void LoginDialog::onForgotPasswordClicked()
{
    ForgotPasswordDialog forgotDialog(this);
    forgotDialog.exec();
}

// ========== 记住我功能实现 ==========
void LoginDialog::loadRememberedCredentials()
{
    QSettings settings("FlyOver", "FlightTicketSystem");
    
    bool remembered = settings.value("login/remember", false).toBool();
    QString savedAccount = settings.value("login/account", "").toString();
    
    if (remembered && !savedAccount.isEmpty()) {
        ui->accountEdit->setText(savedAccount);
        
        // 设置记住我复选框状态
        if (m_rememberCheck) {
            m_rememberCheck->setChecked(true);
        }
        
        // 将焦点设置到密码框
        ui->passwordEdit->setFocus();
    }
}

void LoginDialog::saveCredentials()
{
    QSettings settings("FlyOver", "FlightTicketSystem");
    
    settings.setValue("login/remember", true);
    settings.setValue("login/account", ui->accountEdit->text().trimmed());
    // 注意：出于安全考虑，不保存密码，只保存账号
    
    settings.sync();
}

void LoginDialog::clearCredentials()
{
    QSettings settings("FlyOver", "FlightTicketSystem");
    
    settings.setValue("login/remember", false);
    settings.remove("login/account");
    
    settings.sync();
}

void LoginDialog::toggleAdminCodeInput()
{
    qDebug() << "[LoginDialog] toggleAdminCodeInput 被调用";
    
    // 使用简洁的输入对话框，明确设置父窗口和模态属性
    QInputDialog dialog(this);
    dialog.setWindowTitle("管理员验证");
    dialog.setLabelText("请输入管理员验证码：");
    dialog.setTextEchoMode(QLineEdit::Password);
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setWindowModality(Qt::ApplicationModal);
    
    // 设置对话框样式
    dialog.setStyleSheet(R"(
        QInputDialog {
            background-color: white;
        }
        QLabel {
            color: #333;
            font-size: 14px;
            padding: 10px;
        }
        QLineEdit {
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 14px;
            background-color: white;
        }
        QLineEdit:focus {
            border-color: #3498DB;
        }
        QPushButton {
            background-color: #3498DB;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-size: 14px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #1A5276;
        }
    )");
    
    if (dialog.exec() == QDialog::Accepted) {
        QString code = dialog.textValue();
        if (!code.isEmpty()) {
            if (code == "Admin") {
                m_adminCode = code;
                QMessageBox::information(this, "验证成功", "管理员验证码已确认！\n登录后将以管理员身份进入系统。");
                qDebug() << "[LoginDialog] 管理员验证码验证成功";
            } else {
                m_adminCode.clear();
                QMessageBox::warning(this, "验证失败", "管理员验证码错误！\n将以普通用户身份登录。");
                qDebug() << "[LoginDialog] 管理员验证码验证失败";
            }
        }
    } else {
        qDebug() << "[LoginDialog] 用户取消管理员验证";
    }
}
