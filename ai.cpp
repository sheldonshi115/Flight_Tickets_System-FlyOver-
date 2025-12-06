#include "ai.h"
#include "ui_ai.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
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

    // 初始欢迎消息（安全添加）
    QString sestence = QString("您好，我是一个能回答关于航班查询的问题的助手。如果您有关于航班的问题，请告诉我具体的出发地、目的地和日期。");
    safeAddChatItem("AI", sestence);

    // 信号连接（断开旧连接，避免重复连接）
    disconnect(m_netManager, &QNetworkAccessManager::finished, this, &AIQueryWidget::onReplyFinished);
    connect(m_netManager, &QNetworkAccessManager::finished, this, &AIQueryWidget::onReplyFinished);

    disconnect(ui->questionLineEdit, &QLineEdit::returnPressed, this, &AIQueryWidget::onReturnPressed);
    connect(ui->questionLineEdit, &QLineEdit::returnPressed, this, &AIQueryWidget::onReturnPressed);
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
            QString answer = jsonDoc.object()["answer"].toString();
            safeAddChatItem("AI", answer);
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
