#include "ai.h"
#include "ui_ai.h"
#include "flightmanager.h"
#include "dbmanager.h"
#include "flight.h"
#include "order.h"
#include "membersystem.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDate>
#include <QUuid>
#include <QPainter>
#include <QDateTime>
#include <QApplication>
#include <QMetaObject>
#include <QThread>
#include <QScrollBar>
#include <QColor>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QCloseEvent>
#include <QScreen>// 新增：用于绘制圆形头像
#include <QGuiApplication>
#include <QPushButton>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QKeyEvent>
// note: removed QMouseEvent include; mouse drag handled by system title bar
// 1. 气泡绘制：保留并修复你的代码
void ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    QString sender = index.data(SenderRole).toString();
    QString content = index.data(ContentRole).toString();
    qreal opacity = index.data(OpacityRole).toReal();
    if (opacity <= 0) opacity = 0.01;

    QFont font = QApplication::font();
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(QRect(0, 0, option.rect.width() * 0.75, 1000), Qt::TextWordWrap, content);
    QSize bubbleSize = QSize(textRect.width() + 28, textRect.height() + 20);

    const int avatarSize = 40;
    const int avatarSpacing = 10;
    QRect avatarRect;
    QPixmap aiAvatar;

    QRect bubbleRect;
    QLinearGradient bubbleGradient;

    if (sender == "用户") {
        // 用户气泡：在右侧显示用户头像，气泡向左偏移
        // 头像优先来源：委托内部 m_userAvatar -> item DecorationRole -> widget property "userAvatar"
        QPixmap drawUserAvatar;
        QVariant dec = index.data(Qt::DecorationRole);
        if (dec.canConvert<QPixmap>()) drawUserAvatar = dec.value<QPixmap>();
        if (drawUserAvatar.isNull() && !m_userAvatar.isNull()) drawUserAvatar = m_userAvatar;
        if (drawUserAvatar.isNull()) {
            const QObject *p = option.widget ? option.widget->parent() : nullptr;
            if (p && p->property("userAvatar").isValid()) drawUserAvatar = p->property("userAvatar").value<QPixmap>();
        }

        QRect userAvatarRect(
            option.rect.right() - avatarSize - 10,
            option.rect.top() + (bubbleSize.height() - avatarSize) / 2 + 10,
            avatarSize,
            avatarSize
            );

        bubbleRect = QRect(option.rect.right() - bubbleSize.width() - avatarSize - avatarSpacing,
                           option.rect.top() + 10,
                           bubbleSize.width(),
                           bubbleSize.height());

        bubbleGradient = QLinearGradient(bubbleRect.topLeft(), bubbleRect.bottomRight());
        bubbleGradient.setColorAt(0, QColor(66, 153, 225, int(opacity * 255)));
        bubbleGradient.setColorAt(1, QColor(125, 184, 255, int(opacity * 255)));

        // 绘制用户头像（圆形）
        painter->save();
        QPainterPath path;
        path.addEllipse(userAvatarRect);
        painter->setClipPath(path);
        if (!drawUserAvatar.isNull()) {
            painter->drawPixmap(userAvatarRect, drawUserAvatar.scaled(avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            painter->setBrush(QColor(200, 200, 200));
            painter->drawEllipse(userAvatarRect);
        }
        painter->setClipRect(option.rect);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255,255,255),2));
        painter->drawEllipse(userAvatarRect);
        painter->restore();

    } else {
        // AI 气泡：在左侧显示 AI 头像，气泡右移
        static QPixmap cachedAvatar;
        if (cachedAvatar.isNull()) {
            cachedAvatar = QPixmap(":/resources/images/ai_avatar.png").scaled(avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        aiAvatar = cachedAvatar;

        avatarRect = QRect(option.rect.left() + 10,
                           option.rect.top() + (bubbleSize.height() - avatarSize) / 2 + 10,
                           avatarSize,
                           avatarSize);

        painter->save();
        QPainterPath avatarPath;
        avatarPath.addEllipse(avatarRect);
        painter->setClipPath(avatarPath);
        if (!aiAvatar.isNull()) painter->drawPixmap(avatarRect, aiAvatar);
        else { painter->setBrush(QColor(220,220,220)); painter->drawEllipse(avatarRect); }
        painter->setClipRect(option.rect);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255,255,255),2));
        painter->drawEllipse(avatarRect);
        painter->restore();

        bubbleRect = QRect(option.rect.left() + avatarSize + avatarSpacing + 10,
                           option.rect.top() + 10,
                           bubbleSize.width(),
                           bubbleSize.height());

        bubbleGradient = QLinearGradient(bubbleRect.topLeft(), bubbleRect.bottomRight());
        bubbleGradient.setColorAt(0, QColor(240,240,240, int(opacity*255)));
        bubbleGradient.setColorAt(1, QColor(224,224,224, int(opacity*255)));
    }

    // 绘制气泡与文字
    painter->setBrush(bubbleGradient);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, 20, 20);

    painter->setPen(QColor(45,55,72, int(opacity*255)));
    painter->setFont(font);
    painter->drawText(QRect(bubbleRect.x() + 14, bubbleRect.y() + 10, bubbleRect.width() - 28, bubbleRect.height() - 20),
                      Qt::TextWordWrap, content);

    painter->restore();
}
// 将 show/close 动画实现放在 paint 之外，避免干扰绘制逻辑
void AIQueryWidget::showWithAnimation() {
    if (m_isClosing) return;
    this->setWindowOpacity(0.0);

    // 计算目标几何：始终根据 m_ownerWindow（优先）或主屏幕可用区域居中
    QRect refRect;
    if (m_ownerWindow) {
        refRect = m_ownerWindow->geometry();
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        refRect = screen ? screen->availableGeometry() : QRect(0,0,1024,768);
    }

    // 增大默认尺寸以满足“适当调大一些”的要求
    int w = qMax(720, refRect.width() / 2);
    int h = qMax(520, refRect.height() / 2);
    QRect targetGeom = QRect(refRect.center().x() - w/2, refRect.center().y() - h/2, w, h);

    // 起始为目标中心小尺寸（从中心放大），避免太大或为0
    QRect startGeom;
    startGeom.setSize(QSize(qMax(1, targetGeom.width()/8), qMax(1, targetGeom.height()/8)));
    startGeom.moveCenter(targetGeom.center());

    // 先将窗口放在起始位置再 show，这样不会先闪到左上角
    this->setWindowFlags(windowFlags() | Qt::Window);
    this->setGeometry(startGeom);
    this->show();
    this->raise();
    this->activateWindow();

    QPropertyAnimation *geomAnim = new QPropertyAnimation(this, "geometry");
    geomAnim->setDuration(380);
    geomAnim->setStartValue(startGeom);
    geomAnim->setEndValue(targetGeom);
    geomAnim->setEasingCurve(QEasingCurve::OutBack);

    QPropertyAnimation *opAnim = new QPropertyAnimation(this, "windowOpacity");
    opAnim->setDuration(300);
    opAnim->setStartValue(0.0);
    opAnim->setEndValue(1.0);
    opAnim->setEasingCurve(QEasingCurve::Linear);

    geomAnim->start(QAbstractAnimation::DeleteWhenStopped);
    opAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AIQueryWidget::animateClose() {
    if (m_isClosing) return;
    m_isClosing = true;

    QRect startGeom = this->geometry();

    // 优先尝试定位到父窗口的 AI 按钮（objectName: "btnAIService"），若找到则把目标设为该按钮中心
    QRect endGeom;
    QWidget *parentWin = m_ownerWindow ? m_ownerWindow : this->parentWidget();
    QPushButton *targetBtn = nullptr;
    if (parentWin) targetBtn = parentWin->findChild<QPushButton*>("btnAIService");
    if (targetBtn) {
        QPoint btnCenterGlobal = targetBtn->mapToGlobal(targetBtn->rect().center());
        QSize endSize(qMax(24, startGeom.width()/8), qMax(18, startGeom.height()/8));
        endGeom = QRect(QPoint(0,0), endSize);
        endGeom.moveCenter(btnCenterGlobal);
    } else {
        // 回退：屏幕右下角垃圾桶点
        QRect refRect;
        QWidget *p = this->parentWidget();
        if (p) refRect = p->geometry();
        else {
            QScreen *screen = QGuiApplication::primaryScreen();
            refRect = screen ? screen->availableGeometry() : QRect(0,0,800,600);
        }
        QPoint trashPoint(refRect.right() - 48, refRect.bottom() - 48);
        QSize endSize(qMax(24, startGeom.width()/8), qMax(18, startGeom.height()/8));
        endGeom = QRect(QPoint(0,0), endSize);
        endGeom.moveCenter(trashPoint);
    }

    // 先做一个短暂的“放大”感（paper 投掷前的抬起/突出），再移动到垃圾桶并淡出
    QRect popGeom = startGeom;
    int padW = qMax(10, startGeom.width() / 12);
    int padH = qMax(8, startGeom.height() / 12);
    popGeom.adjust(-padW, -padH, padW, padH); // 稍微放大

    QPropertyAnimation *popAnim = new QPropertyAnimation(this, "geometry");
    popAnim->setDuration(160);
    popAnim->setStartValue(startGeom);
    popAnim->setEndValue(popGeom);
    popAnim->setEasingCurve(QEasingCurve::OutQuad);

    // 移动到垃圾桶并缩小淡出
    QPropertyAnimation *moveAnim = new QPropertyAnimation(this, "geometry");
    moveAnim->setDuration(560);
    moveAnim->setStartValue(popGeom);
    moveAnim->setEndValue(endGeom);
    moveAnim->setEasingCurve(QEasingCurve::InQuad);

    QPropertyAnimation *fadeAnim = new QPropertyAnimation(this, "windowOpacity");
    fadeAnim->setDuration(560);
    fadeAnim->setStartValue(1.0);
    fadeAnim->setEndValue(0.0);
    fadeAnim->setEasingCurve(QEasingCurve::InQuad);

    connect(fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        this->hide();
        this->deleteLater();
    });

    // 顺序动画：pop -> (move + fade)
    QSequentialAnimationGroup *seq = new QSequentialAnimationGroup(this);
    QParallelAnimationGroup *parallel = new QParallelAnimationGroup(seq);
    parallel->addAnimation(moveAnim);
    parallel->addAnimation(fadeAnim);
    seq->addAnimation(popAnim);
    seq->addAnimation(parallel);
    seq->start(QAbstractAnimation::DeleteWhenStopped);
}

// Removed custom mouse drag handlers to rely on system title bar for window movement

void AIQueryWidget::closeEvent(QCloseEvent *event) {
    if (m_isClosing) {
        event->accept();
        return;
    }
    event->ignore();
    animateClose();
}

void AIQueryWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
        return;
    }
    QWidget::keyPressEvent(event);
}
// 新增：更新气泡透明度，实现淡入动画
void AIQueryWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    // 渐变配置：从浅蓝到浅紫（可自行调整颜色）
    QLinearGradient gradient(0, 0, this->width(), this->height());
    gradient.setColorAt(0, QColor(245, 248, 255)); // 左上角颜色（淡蓝）
    gradient.setColorAt(1, QColor(250, 245, 255)); // 右下角颜色（淡紫）
    // 绘制背景
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRect(this->rect());
}
void AIQueryWidget::setUserAvatar(const QPixmap &pixmap) {
    // 将头像传递给委托（若委托提供接口）并作为属性保存以供绘制时读取
    if (m_chatDelegate) {
        // 通过dynamic_cast安全调用（m_chatDelegate是指针）
        ChatDelegate *d = m_chatDelegate;
        if (d) d->setUserAvatar(pixmap);
    }
    // 将头像存为控件属性，paint中会尝试读取该属性作为备选
    this->setProperty("userAvatar", QVariant::fromValue(pixmap));
}
void AIQueryWidget::updateItemOpacity(QListWidgetItem *item) {
    if (!item || !ui->chatListWidget) return;

    qreal opacity = item->data(ChatDelegate::OpacityRole).toReal();
    if (opacity >= 1.0) {
        return;
    }

    opacity += 0.08;
    if (opacity > 1.0) opacity = 1.0;
    item->setData(ChatDelegate::OpacityRole, opacity);

    int interval = 50;

    int row = ui->chatListWidget->row(item);
    if (row != -1) {
        QModelIndex index = ui->chatListWidget->model()->index(row, 0);
        ui->chatListWidget->update(index);
    }

    QTimer::singleShot(interval, this, [=]() {
        updateItemOpacity(item);
    });
}

// 2. Item尺寸计算：完全保留你的代码
QSize ChatDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QString content = index.data(ContentRole).toString();
    QFontMetrics fm(QApplication::font());
    QRect textRect = fm.boundingRect(
        QRect(0, 0, option.rect.width() * 0.75, 1000),
        Qt::TextWordWrap,
        content
        );
    return QSize(option.rect.width(), textRect.height() + 50);
}

// 新增：3. 安全删除"正在思考"Item（强制主线程）
void AIQueryWidget::safeRemoveLoadingItem() {
    // 双重空指针判断，杜绝野指针
    if (this->thread() != QThread::currentThread()) {
        // 不在主线程，投递给主线程
        QMetaObject::invokeMethod(this, &AIQueryWidget::safeRemoveLoadingItem, Qt::QueuedConnection);
        return;
    }

    if (m_loadingItem && ui->chatListWidget->itemWidget(m_loadingItem) == nullptr) {
        // 确认Item属于当前ListWidget，再删除
        int index = ui->chatListWidget->row(m_loadingItem);
        if (index != -1) {
            delete ui->chatListWidget->takeItem(index); // 先从ListWidget移除，再删除
        }
        m_loadingItem = nullptr; // 置空，避免野指针
    }
}

// 新增：4. 安全添加聊天Item（强制主线程）
// 修改：在安全添加Item时，初始化透明度并启动淡入动画
void AIQueryWidget::safeAddChatItem(const QString &sender, const QString &content) {
    if (this->thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, [this, sender, content]() {
            this->safeAddChatItem(sender, content);
        }, Qt::QueuedConnection);
        return;
    }

    if (!ui->chatListWidget) return;

    QListWidgetItem *item = new QListWidgetItem(ui->chatListWidget);
    item->setData(ChatDelegate::SenderRole, sender);
    item->setData(ChatDelegate::ContentRole, content);
    // 🔴 初始化透明度为0.0（淡入动画起点：完全透明）
    item->setData(ChatDelegate::OpacityRole, 0.0);
    ui->chatListWidget->addItem(item);
    ui->chatListWidget->update();

    // 🔴 启动淡入动画（延迟10ms，避免与UI刷新冲突）
    QTimer::singleShot(10, this, [=]() {
        updateItemOpacity(item);
    });

    safeScrollToBottom(); // 原有滚动逻辑不变
}

// 新增：5. 安全滚动到底部（强制主线程）
void AIQueryWidget::safeScrollToBottom() {
    if (this->thread() != QThread::currentThread()) {
        QMetaObject::invokeMethod(this, &AIQueryWidget::safeScrollToBottom, Qt::QueuedConnection);
        return;
    }

    if (ui->chatListWidget) {
        int count = ui->chatListWidget->count();
        if (count > 0) {
            ui->chatListWidget->scrollToItem(ui->chatListWidget->item(count - 1), QAbstractItemView::PositionAtBottom);
        }
    }
}

// 6. 取消当前请求（强化资源释放）
void AIQueryWidget::cancelCurrentRequest() {
    // 网络请求释放
    if (m_currentReply) {
        m_currentReply->disconnect();
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    if (m_requestTimer) {
        m_requestTimer->stop();
    }

    // 安全删除“正在思考”Item
    safeRemoveLoadingItem();

    // 🔴 强制重置请求状态
    m_isRequesting = false;
}
// 7. 超时处理（安全提示）
void AIQueryWidget::onRequestTimeout() {
    cancelCurrentRequest();
    safeAddChatItem("AI", "查询超时！请检查网络或稍后重试~");
}

// 8. 窗口初始化（强化控件安全）
AIQueryWidget::AIQueryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AIQueryWidget),
    m_netManager(new QNetworkAccessManager(this)),
    m_apiUrl("http://localhost:8000/query_flight"),
    m_chatDelegate(new ChatDelegate(this))
{
    ui->setupUi(this);
    this->setWindowTitle("AI assistant");
    // 确保作为顶级窗口时拥有窗口装饰（最大化/最小化/关闭按钮）
    this->setWindowFlag(Qt::Window, true);
    ui->queryButton->setStyleSheet(R"(
        QPushButton {
            color: black;           /* 按钮字体黑色 */
            background-color: #f0f0f0; /* 按钮背景浅灰（可选） */
            border: 1px solid #ccc;    /* 按钮边框（可选） */
            padding: 5px 15px;         /* 按钮内边距（可选） */
        }
        QPushButton:hover { /* 鼠标悬浮样式（可选） */
            background-color: #e0e0e0;
        }
        QPushButton:pressed { /* 点击样式（可选） */
            background-color: #d0d0d0;
        }
    )");
    // 你的原始配置，添加刷新确保初始化正常
    ui->chatListWidget->setItemDelegate(m_chatDelegate);
    ui->chatListWidget->setSpacing(10);
    ui->chatListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    QScrollBar *vScrollBar = ui->chatListWidget->verticalScrollBar();
    if (vScrollBar) {
        vScrollBar->setSingleStep(10);  // 鼠标滚轮/箭头键单次滚动10像素（越小越细腻）
        vScrollBar->setPageStep(80);    // 点击滚动条空白处单次滚动80像素（适配屏幕高度）
    }
    ui->chatListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->chatListWidget->update(); // 初始化后刷新
    ui->chatListWidget->setStyleSheet("background-color: transparent; border: none;");
    // 可选：设置输入框和按钮区域背景透明（保持风格统一）
    ui->questionLineEdit->setStyleSheet(R"(
        QLineEdit, QTextEdit, QPlainTextEdit {
            color: black!important;       /* 输入字体黑色 */
            background-color: white; /* 背景白色（可选，避免全局背景白） */
            border: 1px solid #ccc;  /* 边框（可选，更清晰） */
            padding: 5px;            /* 内边距（可选） */
        }
        QLineEdit:focus { /* 聚焦时样式（可选） */
            border-color: #66afe9;
            outline: none;
        }
    )");
    // 超时计时器（强化空指针判断）
    m_requestTimer = new QTimer(this);
    m_requestTimer->setSingleShot(true);
    m_requestTimer->setInterval(30000);
    connect(m_requestTimer, &QTimer::timeout, this, &AIQueryWidget::onRequestTimeout);

    const QByteArray apiEnv = qgetenv("AI_AGENT_API_URL");
    if (!apiEnv.isEmpty()) {
        m_apiUrl = QString::fromUtf8(apiEnv);
    }

    // 初始欢迎消息（安全添加）
    QString sestence = QString("您好，我是一个能回答关于航班查询的问题的助手。如果您有关于航班的问题，请告诉我具体的出发地、目的地和日期。");
    safeAddChatItem("AI", sestence);

    // 信号连接（断开旧连接，避免重复连接）
    disconnect(m_netManager, &QNetworkAccessManager::finished, this, &AIQueryWidget::onReplyFinished);
    connect(m_netManager, &QNetworkAccessManager::finished, this, &AIQueryWidget::onReplyFinished);

    disconnect(ui->questionLineEdit, &QLineEdit::returnPressed, this, &AIQueryWidget::onReturnPressed);
    connect(ui->questionLineEdit, &QLineEdit::returnPressed, this, &AIQueryWidget::onReturnPressed);

    // 连接模式切换按钮
    connect(ui->btnToggleMode, &QPushButton::clicked, this, &AIQueryWidget::onModeToggleClicked);
}

// 9. 析构函数（彻底释放UI资源）
AIQueryWidget::~AIQueryWidget() {
    // 顺序1：先停止计时器（子资源）
    if (m_requestTimer) {
        m_requestTimer->stop();
        m_requestTimer->deleteLater();
        m_requestTimer = nullptr;
    }

    // 顺序2：取消网络请求（子资源）
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->disconnect();
        m_currentReply->deleteLater();
        m_currentReply = nullptr; // 普通指针置空
    }

    // 顺序3：释放聊天Item（子资源，依赖chatListWidget）
    if (ui && ui->chatListWidget) {
        // 终极方案：循环删除第一个Item，直到没有Item为止（适配所有Qt版本）
        while (true) {
            QListWidgetItem* item = ui->chatListWidget->item(0); // 获取第一个Item
            if (!item) break; // 没有Item了，退出循环
            delete item; // 删除Item
        }
        ui->chatListWidget->clear(); // 最后清空列表
    }

    // 顺序4：释放网络管理器（子资源）
    if (m_netManager) {
        m_netManager->disconnect();
        m_netManager->deleteLater();
        m_netManager = nullptr;
    }

    // 顺序5：释放委托（子资源）
    delete m_chatDelegate;
    m_chatDelegate = nullptr;

    // 顺序6：最后释放UI（父资源，包含chatListWidget）
    delete ui;
    ui = nullptr;
}

// 10. 点击查询按钮（强化安全判断）
void AIQueryWidget::on_queryButton_clicked() {
    // 防止窗口控件已销毁
    if (!ui->questionLineEdit || !ui->chatListWidget) return;

    QString question = ui->questionLineEdit->text().trimmed();
    if (question.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入查询问题！");
        return;
    }

    // 🔴 先添加用户气泡（无论请求是否能发，先显示用户输入）
    safeAddChatItem("用户", question);

    // Agent模式下的本地状态机处理（非空闲状态）
    if (m_isAgentMode && m_flightManager && m_agentState != Idle) {
        ui->questionLineEdit->clear();

        if (m_agentState == WaitingFlight) {
            // 等待选择航班
            bool ok;
            int selection = question.toInt(&ok);
            if (ok && selection > 0 && selection <= m_searchedFlights.size()) {
                m_selectedFlightIndex = selection - 1;
                const Flight &f = m_searchedFlights[m_selectedFlightIndex];

                // 提示选择座位
                safeAddChatItem("AI", QString("您选择了航班 %1\n%2 → %3\n\n请选择座位号（例如：1A, 2B等）\n或输入「随机」让我帮您选择").arg(f.flightNumber()).arg(f.departureCity()).arg(f.arrivalCity()));
                m_agentState = WaitingSeat;
                return;
            } else {
                // 尝试重新解析查询
                QString departure, arrival, date;
                if (parseFlightQuery(question, departure, arrival, date)) {
                    QDateTime dateTime;
                    if (!date.isEmpty()) {
                        dateTime = QDateTime::fromString(date + " 00:00:00", "yyyy-MM-dd hh:mm:ss");
                    }
                    QList<Flight> flights = DBManager::instance().findFlights(departure, arrival, dateTime);
                    showFlightOptions(flights);
                } else {
                    safeAddChatItem("AI", "请输入有效的航班编号（1-" + QString::number(m_searchedFlights.size()) + "）");
                }
                return;
            }
        } else if (m_agentState == WaitingSeat) {
            // 处理座位选择
            if (question.contains("随机") || question.toLower() == "random") {
                m_selectedSeat = "1A"; // 简单演示，随机选一个
            } else {
                m_selectedSeat = question.trimmed().toUpper();
            }

            const Flight &f = m_searchedFlights[m_selectedFlightIndex];
            QString confirmMsg = QString("请确认购票信息:\n\n航班: %1\n路线: %2 → %3\n座位: %4\n票价: ¥%5\n\n回复「确认」购买，或「取消」返回").arg(f.flightNumber()).arg(f.departureCity()).arg(f.arrivalCity()).arg(m_selectedSeat).arg(f.price(), 0, 'f', 2);

            safeAddChatItem("AI", confirmMsg);
            m_agentState = WaitingConfirm;
            return;
        } else if (m_agentState == WaitingConfirm) {
            // 确认购票
            if (question.contains("确认") || question.toLower() == "yes" || question.toLower() == "y") {
                executeBooking();
                return;
            } else if (question.contains("取消") || question.toLower() == "no" || question.toLower() == "n") {
                safeAddChatItem("AI", "已取消购票，如需重新开始请告诉我。");
                m_agentState = Idle;
                m_searchedFlights.clear();
                m_selectedFlightIndex = -1;
                m_selectedSeat.clear();
                return;
            } else {
                safeAddChatItem("AI", "请回复「确认」或「取消」");
                return;
            }
        }

        return;
    }

    // 再检查请求状态
    if (m_isRequesting) {
        QMessageBox::information(this, "提示", "正在处理您的请求，请稍候~");
        ui->questionLineEdit->clear(); // 清空输入框
        return;
    }
    m_isRequesting = true; // 标记为正在处理

    // 添加“正在思考”Item
    QMetaObject::invokeMethod(this, [=]() {
        m_loadingItem = new QListWidgetItem(ui->chatListWidget);
        m_loadingItem->setData(ChatDelegate::SenderRole, "AI");
        m_loadingItem->setData(ChatDelegate::ContentRole, "正在思考.........");
        ui->chatListWidget->addItem(m_loadingItem);
        ui->chatListWidget->update();
        safeScrollToBottom();
    }, Qt::QueuedConnection);

    // 🔴 确保发送请求的代码被执行（后端没收到请求的核心原因）
    QJsonObject jsonObj;
    jsonObj["question"] = question;
    QJsonDocument jsonDoc(jsonObj);
    QByteArray postData = jsonDoc.toJson();

    QNetworkRequest request;
    request.setUrl(QUrl(m_apiUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 重置当前请求（避免旧请求干扰）
    if (m_currentReply) {
        m_currentReply->disconnect();
        m_currentReply->deleteLater();
    }
    m_currentReply = m_netManager->post(request, postData);
    m_requestTimer->start(); // 启动超时计时器

    ui->questionLineEdit->clear();
}

// 11. 网络响应处理（所有UI操作走安全函数）
void AIQueryWidget::onReplyFinished(QNetworkReply *reply) {


    // 🔴 第二步：检查关键控件是否存在
    if (!ui || !ui->chatListWidget || !ui->queryButton) {
        reply->deleteLater();
        return;
    }
    if (m_requestTimer) {
        m_requestTimer->stop();
    }

    // 安全删除“正在思考”Item
    safeRemoveLoadingItem();

    // 处理响应
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            safeAddChatItem("AI", QString("查询失败：%1").arg(reply->errorString()));
        }
    } else {
        QByteArray response = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        if (!jsonDoc.isObject()) {
            safeAddChatItem("AI", "查询失败：无效响应");
        } else {
            QJsonObject obj = jsonDoc.object();
            QString answer = obj.value("answer").toString();
            QJsonObject agentObj = obj.value("agent").toObject();

            // Agent模式下的特殊处理
            if (m_isAgentMode) {
                processAgentResponse(agentObj, answer);
            } else {
                safeAddChatItem("AI", answer);
            }
        }
    }

    // 🔴 强制重置请求状态
    m_currentReply = nullptr;
    m_isRequesting = false; // 无论成功失败，都重置为false

    // 释放reply
    reply->disconnect();
    reply->deleteLater();
}
// 12. 回车触发：保留你的代码
void AIQueryWidget::onReturnPressed() {
    on_queryButton_clicked();
}

// 设置FlightManager引用
void AIQueryWidget::setFlightManager(FlightManager *manager) {
    m_flightManager = manager;
}

// 设置当前用户账号
void AIQueryWidget::setCurrentUser(const QString &account) {
    m_currentUserAccount = account;
}

// 模式切换槽函数
void AIQueryWidget::onModeToggleClicked() {
    m_isAgentMode = !m_isAgentMode;

    // 重置Agent状态
    m_agentState = Idle;
    m_searchedFlights.clear();
    m_selectedFlightIndex = -1;
    m_selectedSeat.clear();

    if (m_isAgentMode) {
        ui->lblMode->setText("当前模式: Agent模式");
        ui->btnToggleMode->setText("切换到普通模式");
        ui->lblMode->setStyleSheet("font-weight: bold; color: #4CAF50; padding: 5px;");

        if (!m_flightManager) {
            safeAddChatItem("系统", "Agent模式需要连接FlightManager，请稍后再试。");
            m_isAgentMode = false;
            ui->lblMode->setText("当前模式: 普通模式");
            ui->btnToggleMode->setText("切换到Agent模式");
            return;
        }

        safeAddChatItem("AI", "已切换到Agent模式！\n\n我可以帮您:\n1. 查询航班\n2. 选择座位\n3. 购买机票\n\n请告诉我您的需求，例如：\n「我想买从北京到上海的机票」\n「查询明天去广州的航班」");
    } else {
        ui->lblMode->setText("当前模式: 普通模式");
        ui->btnToggleMode->setText("切换到Agent模式");
        ui->lblMode->setStyleSheet("font-weight: bold; color: #2196F3; padding: 5px;");
        safeAddChatItem("AI", "已切换到普通模式，现在我只能回答航班查询相关问题。");
    }
}

// Agent响应处理（依赖后端模型返回的结构化数据）
void AIQueryWidget::processAgentResponse(const QJsonObject &agentData, const QString &answer) {
    // 展示模型原始回复，保持与后端一致的反馈
    if (!answer.trimmed().isEmpty()) {
        safeAddChatItem("AI", answer);
    }

    const bool isFlightQuery = agentData.value("is_flight_query").toBool(false);
    if (!isFlightQuery) {
        // 非航班查询，重置状态
        m_agentState = Idle;
        m_searchedFlights.clear();
        m_selectedFlightIndex = -1;
        m_selectedSeat.clear();
        return;
    }

    const QString departure = agentData.value("departure").toString();
    const QString destination = agentData.value("destination").toString();
    const QString timeType = agentData.value("time_type").toString();
    const QString specificDate = agentData.value("specific_date").toString();

    // 允许模糊查询：出发地和目的地都可以为空
    if (departure.isEmpty() && destination.isEmpty()) {
        safeAddChatItem("AI", "请至少告诉我出发地或目的地之一，以便查询航班。");
        m_agentState = Idle;
        m_searchedFlights.clear();
        m_selectedFlightIndex = -1;
        m_selectedSeat.clear();
        return;
    }

    QDate startDate;
    QDate endDate;
    bool useExactDate = false;
    const bool hasDateRange = resolveAgentDateRange(timeType, specificDate, startDate, endDate, useExactDate);

    QJsonArray flightsArray = agentData.value("flights").toArray();
    QList<Flight> flights = convertAgentFlights(flightsArray);

    // 如模型未返回航班明细，则回退到本地数据库查询
    if (flights.isEmpty()) {
        QList<Flight> fallback;
        if (useExactDate && hasDateRange) {
            QDateTime queryDate(startDate, QTime(0, 0, 0));
            fallback = DBManager::instance().findFlights(departure, destination, queryDate);
        } else {
            fallback = DBManager::instance().findFlights(departure, destination, QDateTime());
            if (hasDateRange) {
                QList<Flight> filtered;
                for (const Flight &flt : fallback) {
                    const QDate departDate = flt.departureTime().date();
                    if (departDate >= startDate && departDate <= endDate) {
                        filtered.append(flt);
                    }
                }
                fallback = filtered;
            }
        }
        flights = fallback;
    }

    if (flights.isEmpty()) {
        safeAddChatItem("AI", "本地系统没有找到可供购票的航班，您可以尝试换一个日期或目的地。");
        m_agentState = Idle;
        m_searchedFlights.clear();
        m_selectedFlightIndex = -1;
        m_selectedSeat.clear();
        return;
    }

    // 限制展示数量，与模型返回顺序保持一致（若有）
    if (flights.size() > 10) {
        flights = flights.mid(0, 10);
    }

    m_searchedFlights = flights;
    m_agentState = WaitingFlight;
    m_selectedFlightIndex = -1;
    m_selectedSeat.clear();

    safeAddChatItem("AI", "请输入航班编号（例如：1）以选择航班，或继续描述新的需求。");
}

bool AIQueryWidget::resolveAgentDateRange(const QString &timeType, const QString &specificDate,
                                          QDate &startDate, QDate &endDate, bool &useExactDate) const {
    startDate = QDate();
    endDate = QDate();
    useExactDate = false;

    const QDate today = QDate::currentDate();

    if (timeType == "具体日期" && !specificDate.isEmpty()) {
        const QDate parsed = QDate::fromString(specificDate, "yyyy-MM-dd");
        if (parsed.isValid()) {
            startDate = parsed;
            endDate = parsed;
            useExactDate = true;
            return true;
        }
    } else if (timeType == "今天") {
        startDate = today;
        endDate = today;
        useExactDate = true;
        return true;
    } else if (timeType == "明天") {
        startDate = today.addDays(1);
        endDate = startDate;
        useExactDate = true;
        return true;
    } else if (timeType == "本周") {
        const int weekday = today.dayOfWeek();
        startDate = today.addDays(1 - weekday); // 周一
        endDate = startDate.addDays(6);
        return true;
    } else if (timeType == "下周") {
        const int weekday = today.dayOfWeek();
        startDate = today.addDays(8 - weekday); // 下周一
        endDate = startDate.addDays(6);
        return true;
    } else if (timeType == "本月") {
        startDate = QDate(today.year(), today.month(), 1);
        endDate = startDate.addMonths(1).addDays(-1);
        return true;
    } else if (timeType == "最近") {
        startDate = today;
        endDate = today.addDays(7);
        return true;
    }

    return false;
}

QList<Flight> AIQueryWidget::convertAgentFlights(const QJsonArray &flightArray) const {
    QList<Flight> flights;
    flights.reserve(flightArray.size());

    for (const QJsonValue &value : flightArray) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject obj = value.toObject();
        Flight flight;
        flight.setId(obj.value("id").toInt());
        flight.setFlightNumber(obj.value("flight_num").toString());
        flight.setDepartureCity(obj.value("departure").toString());
        flight.setArrivalCity(obj.value("destination").toString());

        const QString departStr = obj.value("depart_time").toString();
        QDateTime departTime = QDateTime::fromString(departStr, Qt::ISODate);
        if (!departTime.isValid()) {
            departTime = QDateTime::fromString(departStr, "yyyy-MM-dd HH:mm:ss");
        }
        flight.setDepartureTime(departTime);

        const QString arriveStr = obj.value("arrive_time").toString();
        QDateTime arriveTime = QDateTime::fromString(arriveStr, Qt::ISODate);
        if (!arriveTime.isValid()) {
            arriveTime = QDateTime::fromString(arriveStr, "yyyy-MM-dd HH:mm:ss");
        }
        flight.setArrivalTime(arriveTime);

        flight.setTotalSeats(obj.value("seat_count").toInt());
        const int available = obj.value("available_seats").toInt(flight.totalSeats());
        flight.setAvailableSeats(available);
        flight.setPrice(obj.value("price").toDouble());

        if (flight.isValid()) {
            flights.append(flight);
        }
    }

    return flights;
}

// 解析航班查询参数
bool AIQueryWidget::parseFlightQuery(const QString &query, QString &departure, QString &arrival, QString &date) {
    // 简单的规则匹配（实际应该用NLP）
    QStringList cities = {"北京", "上海", "广州", "深圳", "成都", "杭州", "西安", "重庆", "南京", "武汉"};

    for (const QString &city : cities) {
        if (query.contains(city) && departure.isEmpty()) {
            departure = city;
        } else if (query.contains(city) && !departure.isEmpty() && arrival.isEmpty()) {
            arrival = city;
        }
    }

    // 提取日期（简单匹配"今天"、"明天"等）
    if (query.contains("今天")) {
        date = QDate::currentDate().toString("yyyy-MM-dd");
    } else if (query.contains("明天")) {
        date = QDate::currentDate().addDays(1).toString("yyyy-MM-dd");
    }

    return !departure.isEmpty() && !arrival.isEmpty();
}

// 显示航班选项
void AIQueryWidget::showFlightOptions(const QList<Flight> &flights) {
    if (flights.isEmpty()) {
        safeAddChatItem("AI", "抱歉，没有找到符合条件的航班。");
        m_agentState = Idle;
        return;
    }

    m_searchedFlights = flights;
    QString msg = QString("找到 %1 个航班:\n\n").arg(flights.size());

    for (int i = 0; i < flights.size() && i < 5; ++i) {
        const Flight &f = flights[i];
        msg += QString("[%1] %2\n").arg(i + 1).arg(f.flightNumber());
        msg += QString("    %1 → %2\n").arg(f.departureCity()).arg(f.arrivalCity());
        msg += QString("    起飞: %1\n").arg(f.departureTime().toString("yyyy-MM-dd hh:mm"));
        msg += QString("    票价: ¥%1 | 余票: %2\n\n").arg(f.price(), 0, 'f', 2).arg(f.availableSeats());
    }

    msg += "请输入航班编号（如：1）选择航班";
    safeAddChatItem("AI", msg);
    m_agentState = WaitingFlight;
}

// 执行购票
void AIQueryWidget::executeBooking() {
    if (!m_flightManager || m_selectedFlightIndex < 0 ||
        m_selectedFlightIndex >= m_searchedFlights.size() || m_selectedSeat.isEmpty()) {
        safeAddChatItem("AI", "购票信息不完整，请重新开始。");
        m_agentState = Idle;
        return;
    }

    const Flight &flight = m_searchedFlights[m_selectedFlightIndex];

    // 检查座位可用性
    if (flight.availableSeats() <= 0) {
        safeAddChatItem("AI", "抱歉，该航班已无可用座位。");
        m_agentState = Idle;
        return;
    }

    // 检查飞机币余额
    if (!m_currentUserAccount.isEmpty()) {
        MemberSystem& memberSys = MemberSystem::instance();
        MemberInfo memberInfo = memberSys.getMemberInfo(m_currentUserAccount);

        if (memberInfo.balance < flight.price()) {
            safeAddChatItem("AI", QString("抱歉，您的飞机币余额不足！\n当前余额：¥%1\n票价：¥%2")
                                      .arg(memberInfo.balance, 0, 'f', 2)
                                      .arg(flight.price(), 0, 'f', 2));
            m_agentState = Idle;
            return;
        }

        // 扣除飞机币并累积飞行里程
        if (!memberSys.deductBalance(m_currentUserAccount, flight.price(),
                                     QString("通过AI Agent购买航班 %1 座位 %2").arg(flight.flightNumber()).arg(m_selectedSeat))) {
            safeAddChatItem("AI", "扣除飞机币失败，请重试！");
            m_agentState = Idle;
            return;
        }
        
        // 累积飞行里程（按票价计算，1元=1里程）
        memberSys.addMileage(m_currentUserAccount, flight.price());
    }

    // 更新可用座位
    Flight updatedFlight = flight;
    updatedFlight.setAvailableSeats(flight.availableSeats() - 1);
    if (!DBManager::instance().updateFlight(updatedFlight)) {
        // 恢复飞机币
        if (!m_currentUserAccount.isEmpty()) {
            MemberSystem::instance().addBalance(m_currentUserAccount, flight.price(), "购票失败，退款");
        }
        safeAddChatItem("AI", "座位更新失败，请重试！");
        m_agentState = Idle;
        return;
    }

    // 生成订单
    Order newOrder;
    newOrder.setOrderNumber(QString("ORD%1%2").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")).arg(QUuid::createUuid().toString().mid(1, 8)));
    newOrder.setFlightNumber(flight.flightNumber());
    newOrder.setDepartureCity(flight.departureCity());
    newOrder.setArrivalCity(flight.arrivalCity());
    newOrder.setDepartTime(flight.departureTime());
    newOrder.setSeatNumber(m_selectedSeat);
    newOrder.setPrice(flight.price());
    newOrder.setStatus("已支付");

    // 写入数据库
    if (!DBManager::instance().addOrder(newOrder)) {
        // 恢复座位和余额
        updatedFlight.setAvailableSeats(flight.availableSeats());
        DBManager::instance().updateFlight(updatedFlight);

        if (!m_currentUserAccount.isEmpty()) {
            MemberSystem::instance().addBalance(m_currentUserAccount, flight.price(), "订单创建失败，退款");
        }

        safeAddChatItem("AI", "订单创建失败，座位已恢复！");
        m_agentState = Idle;
        return;
    }

    // 标记座位为已售
    DBManager::instance().markSeatAsSold(flight.flightNumber(), m_selectedSeat);

    // 增加飞行里程
    if (!m_currentUserAccount.isEmpty()) {
        // 简单估算：每100公里算1里程
        double distance = 500.0; // 默认距离
        MemberSystem::instance().addMileage(m_currentUserAccount, distance);
    }

    // 购票成功
    QString successMsg = QString("🎉 购票成功！\n\n订单号：%1\n航班：%2\n路线：%3 → %4\n出发时间：%5\n座位：%6\n票价：¥%7\n\n感谢您的购买！")
                             .arg(newOrder.orderNumber())
                             .arg(flight.flightNumber())
                             .arg(flight.departureCity())
                             .arg(flight.arrivalCity())
                             .arg(flight.departureTime().toString("yyyy-MM-dd hh:mm"))
                             .arg(m_selectedSeat)
                             .arg(flight.price(), 0, 'f', 2);

    safeAddChatItem("AI", successMsg);

    // 重置状态
    m_agentState = Idle;
    m_searchedFlights.clear();
    m_selectedFlightIndex = -1;
    m_selectedSeat.clear();
}
