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
#include <QPainterPath> // 新增：用于绘制圆形头像
// 1. 气泡绘制：完全保留你的代码
void ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform); // 新增：图片平滑缩放

    QString sender = index.data(SenderRole).toString();
    QString content = index.data(ContentRole).toString();
    qreal opacity = index.data(OpacityRole).toReal();
    if (opacity <= 0) opacity = 0.01;

    // 原有文字尺寸计算逻辑不变
    QFont font = QApplication::font();
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(QRect(0, 0, option.rect.width() * 0.75, 1000), Qt::TextWordWrap, content);
    QSize bubbleSize = QSize(textRect.width() + 28, textRect.height() + 20);

    // 🔴 头像配置（仅AI气泡显示）
    const int avatarSize = 40; // 头像尺寸（40x40，可调整）
    const int avatarSpacing = 10; // 头像与气泡的间距
    QRect avatarRect; // 头像绘制区域
    QPixmap aiAvatar; // AI头像图片

    QRect bubbleRect;
    QLinearGradient bubbleGradient;
    if (sender == "用户") {
        // 用户气泡：原有逻辑不变（无头像）
        bubbleRect = QRect(option.rect.right() - bubbleSize.width(), option.rect.top() + 10, bubbleSize.width(), bubbleSize.height());
        bubbleGradient = QLinearGradient(bubbleRect.topLeft(), bubbleRect.bottomRight());
        bubbleGradient.setColorAt(0, QColor(66, 153, 225, int(opacity * 255)));
        bubbleGradient.setColorAt(1, QColor(125, 184, 255, int(opacity * 255)));
    } else {
        // 🔴 AI气泡：先绘制头像，再调整气泡位置
        // 1. 加载AI头像（资源文件路径，确保res.qrc配置正确）
        static QPixmap cachedAvatar; // 静态缓存，避免每次绘制都加载
        if (cachedAvatar.isNull()) {
            // 从资源文件加载图片（路径对应res.qrc中的配置）
            cachedAvatar = QPixmap(":/resources/images/ai_avatar.png").scaled(
                avatarSize, avatarSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation // 平滑缩放
                );
        }
        aiAvatar = cachedAvatar;

        // 2. 确定头像位置（左侧居中对齐气泡）
        avatarRect = QRect(
            option.rect.left() + 10, // 头像左侧距离窗口左边10px
            option.rect.top() + (bubbleSize.height() - avatarSize) / 2 + 10, // 垂直居中（与气泡对齐）
            avatarSize,
            avatarSize
            );

        // 3. 绘制圆形头像（裁剪为圆形，抗锯齿）
        painter->save();
        QPainterPath avatarPath;
        avatarPath.addEllipse(avatarRect); // 圆形路径
        painter->setClipPath(avatarPath); // 裁剪为圆形

        // 绘制头像（加载失败时显示默认灰色圆形）
        if (!aiAvatar.isNull()) {
            painter->drawPixmap(avatarRect, aiAvatar);
        } else {
            painter->setBrush(QColor(220, 220, 220));
            painter->drawEllipse(avatarRect);
        }

        // 绘制头像边框（可选：白色细边框，增强立体感）
        painter->setClipRect(option.rect); // 取消裁剪
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(255, 255, 255), 2));// 白色边框，2px宽
        painter->drawEllipse(avatarRect);
        painter->restore();

        // 4. 调整AI气泡位置：向右偏移（头像宽度+间距），避免重叠
        bubbleRect = QRect(
            option.rect.left() + avatarSize + avatarSpacing + 10, // 原有10px + 头像40px + 间距10px
            option.rect.top() + 10,
            bubbleSize.width(),
            bubbleSize.height()
            );

        // 气泡渐变（原有逻辑不变）
        bubbleGradient = QLinearGradient(bubbleRect.topLeft(), bubbleRect.bottomRight());
        bubbleGradient.setColorAt(0, QColor(240, 240, 240, int(opacity * 255)));
        bubbleGradient.setColorAt(1, QColor(224, 224, 224, int(opacity * 255)));
    }

    // 绘制气泡（原有逻辑不变）
    painter->setBrush(bubbleGradient);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, 20, 20);

    // 绘制文字（原有逻辑不变）
    painter->setPen(QColor(45, 55, 72, int(opacity * 255)));
    painter->setFont(font);
    painter->drawText(QRect(bubbleRect.x() + 14, bubbleRect.y() + 10, bubbleRect.width() - 28, bubbleRect.height() - 20),
                      Qt::TextWordWrap, content);

    painter->restore();
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
    safeAddChatItem("AI", "我是一个能回答关于航班查询的问题的助手。如果您有关于航班的问题，请告诉我具体的出发地、目的地和日期。");

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
