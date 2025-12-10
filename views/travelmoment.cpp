#include "travelmoment.h"
#include "ui_travelmoment.h"
#include "clickablelabel.h"
#include<dbmanager.h>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QPixmap>
#include <QLayoutItem>
#include <QInputDialog>

TravelMoment::TravelMoment(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TravelMoment)
    , nextId(1) // 初始化nextId
{
    qDebug() << "开始创建TravelMoment界面...";
    ui->setupUi(this);
    qDebug() << "ui初始化完成，开始加载动态...";
    loadMomentsFromDB();
    qDebug() << "动态加载完成，开始刷新列表...";
    refreshList();
    qDebug() << "TravelMoment界面创建完成！";
}

void TravelMoment::loadMomentsFromDB() {
    qDebug() << "开始加载数据库中的动态...";
    moments = DBManager::instance().getAllMoments();
    qDebug() << "加载完成，动态数量：" << moments.size();

    if (!moments.isEmpty()) {
        int maxId = 0;
        for (const auto& m : moments) {
            if (m.id > maxId) maxId = m.id;
        }
        nextId = maxId + 1;
        qDebug() << "下一个动态ID：" << nextId;
    } else {
        nextId = 1; // 无数据时重置为1
    }
}

TravelMoment::~TravelMoment()
{
    delete ui;
}

void TravelMoment::addMoment(const MomentItem &item)
{
    qDebug() << "添加动态：" << item.content;

    // 创建卡片（保持原有深色风格，不修改）
    QWidget *card = new QWidget;
    card->setStyleSheet("background-color: #2a2a2a; border-radius: 8px; padding: 10px; margin: 5px;");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(6);
    cardLayout->setContentsMargins(10, 10, 10, 10);

    // 用户名 + 时间
    QHBoxLayout *headerLayout = new QHBoxLayout;
    QLabel *userLabel = new QLabel(item.userName);
    userLabel->setStyleSheet("color: #4fc3f7; font-weight: bold; font-size: 14px;");
    QLabel *timeLabel = new QLabel(item.publishTime.toString("yyyy-MM-dd HH:mm"));
    timeLabel->setStyleSheet("color: #999; font-size: 12px;");
    headerLayout->addWidget(userLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(timeLabel);
    cardLayout->addLayout(headerLayout);

    // 内容
    QLabel *contentLabel = new QLabel(item.content);
    contentLabel->setWordWrap(true);
    contentLabel->setStyleSheet("color: white; font-size: 14px; margin-top: 5px;");
    cardLayout->addWidget(contentLabel);

    // 图片（支持多张，但先显示第一张）
    if (!item.images.isEmpty()) {
        ClickableLabel *imageLabel = new ClickableLabel();
        QPixmap pixmap(item.images.first());
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            imageLabel->setPixmap(pixmap);
            imageLabel->setAlignment(Qt::AlignCenter);
            imageLabel->setStyleSheet("margin-top: 8px; border: 1px solid #444;");

            // 点击图片预览
            imageLabel->setCursor(Qt::PointingHandCursor);
            imageLabel->setProperty("fullImagePath", item.images.first());
            connect(imageLabel, &ClickableLabel::clicked, this, [this, path = item.images.first()]() {
                onImageClicked(path);
            });
        } else {
            qDebug() << "⚠️ 图片加载失败：" << item.images.first();
            QLabel *errLabel = new QLabel("图片加载失败");
            errLabel->setStyleSheet("color: red; font-style: italic;");
            cardLayout->addWidget(errLabel);
        }
        cardLayout->addWidget(imageLabel);
    }

    // 点赞/评论（简化）
    QHBoxLayout *actionLayout = new QHBoxLayout;
    QPushButton *likeBtn = new QPushButton(QString("👍 %1").arg(item.likeCount));
    QPushButton *commentBtn = new QPushButton(QString("💬 %1").arg(item.commentCount));
    likeBtn->setFlat(true);
    commentBtn->setFlat(true);
    likeBtn->setStyleSheet("color: #ff9800; text-align: left;");
    commentBtn->setStyleSheet("color: #4caf50; text-align: left;");
    actionLayout->addWidget(likeBtn);
    actionLayout->addWidget(commentBtn);
    actionLayout->addStretch();
    cardLayout->addLayout(actionLayout);

    // 添加到UI定义的momentListLayout（保持原有逻辑）
    ui->momentListLayout->addWidget(card);
}

// 核心修复：refreshList改为操作momentListLayout，而非替换根布局
void TravelMoment::refreshList()
{
    // 检查UI控件是否初始化
    if (!ui || !ui->momentListLayout) {
        qCritical() << "refreshList：UI控件未初始化";
        return;
    }

    // 清空momentListLayout的旧布局项（安全删除）
    QLayoutItem *it;
    while ((it = ui->momentListLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = it->widget()) {
            w->deleteLater();
        } else if (QLayout *sub = it->layout()) {
            QLayoutItem *subIt;
            while ((subIt = sub->takeAt(0)) != nullptr) {
                if (QWidget *sw = subIt->widget()) sw->deleteLater();
                delete subIt;
            }
            delete sub;
        }
        delete it;
    }

    // 空状态提示
    if (moments.isEmpty()) {
        QLabel *empty = new QLabel("还没有动态，快来发布第一条吧～");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color:#999; padding:40px 0;");
        ui->momentListLayout->addWidget(empty);
        ui->momentListLayout->addStretch();
        return;
    }

    // 逐条创建卡片（统一使用原有深色风格，避免冲突）
    for (const auto &m : moments) {
        // 外层卡片（复用addMoment的深色风格）
        QWidget *card = new QWidget;
        card->setStyleSheet("background-color: #2a2a2a; border-radius: 8px; padding: 10px; margin: 5px;");
        QVBoxLayout *cardLay = new QVBoxLayout(card);
        cardLay->setSpacing(6);
        cardLay->setContentsMargins(14,14,14,14);

        // 头部：头像、名称、时间
        QHBoxLayout *header = new QHBoxLayout;
        header->setSpacing(10);

        // 头像（圆形灰色占位，适配深色风格）
        QLabel *avatar = new QLabel;
        avatar->setFixedSize(44,44);
        avatar->setStyleSheet("background:#444; border-radius:22px; color:#ccc; font-size:12px;");
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setText("头像");

        // 名称 + 时间垂直排列（适配深色风格）
        QVBoxLayout *nameTime = new QVBoxLayout;
        nameTime->setSpacing(2);
        QLabel *name = new QLabel(m.userName);
        name->setStyleSheet("color: #4fc3f7; font-weight:600; font-size:14px;");
        QLabel *time = new QLabel(m.publishTime.toString("MM-dd hh:mm"));
        time->setStyleSheet("color: #999; font-size:12px;");
        nameTime->addWidget(name);
        nameTime->addWidget(time);

        header->addWidget(avatar);
        header->addLayout(nameTime);
        header->addStretch();
        cardLay->addLayout(header);

        // 正文（适配深色风格）
        QLabel *content = new QLabel;
        content->setWordWrap(true);
        content->setText(m.content.isEmpty() ? QString() : m.content);
        content->setStyleSheet("color: white; font-size:14px; line-height:1.4;");
        content->setMinimumHeight(20);
        cardLay->addWidget(content);

        // 图片网格（如果有）
        if (!m.images.isEmpty()) {
            QGridLayout *grid = new QGridLayout;
            grid->setSpacing(6);
            int cnt = qMin(9, m.images.size());
            for (int i = 0; i < cnt; ++i) {
                QLabel *img = new QLabel;
                img->setFixedSize(100,100);
                img->setScaledContents(true);
                QPixmap p(m.images[i]);
                if (!p.isNull()) {
                    img->setPixmap(p.scaled(img->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                } else {
                    img->setStyleSheet("background:#444; border:1px solid #666; color:#ccc;");
                    img->setText("图片");
                    img->setAlignment(Qt::AlignCenter);
                }
                grid->addWidget(img, i/3, i%3);
            }
            cardLay->addLayout(grid);
        }

        // 底部操作栏（点赞/评论，适配深色风格）
        QFrame *sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        sep->setStyleSheet("color:#666;");
        cardLay->addWidget(sep);

        QHBoxLayout *footer = new QHBoxLayout;
        footer->setSpacing(16);

        // like 按钮风格（适配深色）
        QString likeText = (m.liked ? QString("♥ ") : QString("♡ ")) + QString::number(m.likeCount);
        QPushButton *likeBtn = new QPushButton(likeText);
        likeBtn->setFlat(true);
        likeBtn->setCursor(Qt::PointingHandCursor);
        likeBtn->setStyleSheet("QPushButton{color:#ff4d4f; font-size:13px;} QPushButton:pressed{opacity:0.8;}");

        QPushButton *commentBtn = new QPushButton(QString("评论 ") + QString::number(m.commentCount));
        commentBtn->setFlat(true);
        commentBtn->setCursor(Qt::PointingHandCursor);
        commentBtn->setStyleSheet("QPushButton{color:#4caf50; font-size:13px;}");

        footer->addStretch();
        footer->addWidget(likeBtn);
        footer->addWidget(commentBtn);

        cardLay->addLayout(footer);

        // 修复按钮绑定：避免捕获循环变量m
        int momentId = m.id;
        bool currLiked = m.liked;
        int currLikeCount = m.likeCount;
        connect(likeBtn, &QPushButton::clicked, this, [this, momentId, likeBtn, currLiked, currLikeCount]() {
            // 1. 同步到数据库
            bool newLiked = !currLiked;
            DBManager::instance().updateMomentLike(momentId, newLiked);
            // 2. 更新内存数据
            for (auto &mm : moments) {
                if (mm.id == momentId) {
                    mm.liked = newLiked;
                    mm.likeCount += newLiked ? 1 : -1;
                    QString txt = (mm.liked ? QString("♥ ") : QString("♡ ")) + QString::number(mm.likeCount);
                    likeBtn->setText(txt);
                    break;
                }
            }
        });

        connect(commentBtn, &QPushButton::clicked, this, [this, momentId, commentBtn, cardLay]() {
            // 弹出评论输入框
            bool ok;
            QString content = QInputDialog::getMultiLineText(this, "发表评论", "请输入评论内容：", "", &ok);
            if (ok && !content.trimmed().isEmpty()) {
                // 保存评论到数据库
                if (DBManager::instance().addComment(momentId, content, "当前用户")) {
                    // 刷新内存中评论数
                    for (auto& mm : moments) {
                        if (mm.id == momentId) {
                            mm.commentCount += 1;
                            break;
                        }
                    }
                    // 刷新该动态的评论列表
                    refreshComments(momentId, cardLay);
                    // 更新按钮显示的评论数
                    int newCount = commentBtn->text().split(" ")[1].toInt() + 1;
                    commentBtn->setText(QString("评论 ") + QString::number(newCount));
                } else {
                    QMessageBox::warning(this, "错误", "发表评论失败！");
                }
            }
        });

        // 加载并显示该动态的评论
        refreshComments(momentId, cardLay);

        // 添加到UI定义的momentListLayout
        ui->momentListLayout->addWidget(card);
    }

    // 底部拉伸，避免内容贴底
    ui->momentListLayout->addStretch();
}

void TravelMoment::on_selectImageBtn_clicked()
{
    QStringList paths = QFileDialog::getOpenFileNames(
        this,
        "选择图片",
        "",
        "图片文件 (*.png *.jpg *.jpeg *.bmp)"
        );

    if (paths.isEmpty()) return;

    // 限制最多9张
    for (const QString &path : paths) {
        if (selectedImages.size() >= 9) break;
        if (!selectedImages.contains(path)) {
            selectedImages.append(path);
        }
    }

    ui->imagePreview->setText(QString("已选择 %1 张图片").arg(selectedImages.size()));
}

void TravelMoment::onLikeClicked(int id)
{
    // 切换对应 id 的点赞状态并刷新列表显示
    for (auto &mm : moments) {
        if (mm.id == id) {
            mm.liked = !mm.liked;
            mm.likeCount += mm.liked ? 1 : -1;
            break;
        }
    }
    refreshList();
}

void TravelMoment::onCommentClicked(int id)
{
    QMessageBox::information(this, tr("评论"), tr("点击了评论，动态ID：%1").arg(id));
}

void TravelMoment::onImageClicked(const QString &path)
{
    showImagePreview(path);
}

void TravelMoment::showImagePreview(const QString &imagePath)
{
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) return;

    ClickableLabel *preview = new ClickableLabel();
    preview->setPixmap(pixmap.scaled(600, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    preview->setWindowTitle("图片预览 - 点击关闭");
    preview->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    preview->setStyleSheet("background: black;");
    preview->setAlignment(Qt::AlignCenter);
    preview->setAttribute(Qt::WA_DeleteOnClose);

    connect(preview, &ClickableLabel::clicked, preview, &QWidget::close);

    preview->show();
}

void TravelMoment::on_publishBtn_clicked()
{
    QString content = ui->contentEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入内容！");
        return;
    }

    MomentItem newItem;
    newItem.id = nextId++;
    newItem.content = content;
    newItem.images = selectedImages;
    newItem.publishTime = QDateTime::currentDateTime();
    newItem.userName = "当前用户";
    newItem.likeCount = 0;
    newItem.commentCount = 0;
    newItem.liked = false;

    // 保存到数据库
    if (DBManager::instance().addMoment(newItem)) {
        moments.append(newItem);
        refreshList(); // 改用refreshList统一渲染
    } else {
        QMessageBox::warning(this, "错误", "发布动态失败！");
    }

    // 清空
    ui->contentEdit->clear();
    selectedImages.clear();
    ui->imagePreview->setText("已选择 0 张图片");
}

// 修复：增加cardLayout空指针检查
void TravelMoment::refreshComments(int momentId, QVBoxLayout* cardLayout) {
    if (!cardLayout) {
        qCritical() << "refreshComments：cardLayout是空指针！";
        return;
    }

    // 1. 先移除旧评论区
    QLayoutItem* commentItem = nullptr;
    for (int i = 0; i < cardLayout->count(); ++i) {
        QLayoutItem* item = cardLayout->itemAt(i);
        if (item->widget() && item->widget()->objectName() == QString("commentArea_%1").arg(momentId)) {
            commentItem = item;
            break;
        }
    }
    if (commentItem) {
        commentItem->widget()->deleteLater();
        cardLayout->removeItem(commentItem);
        delete commentItem;
    }

    // 2. 添加新评论区（适配深色风格）
    QWidget* commentWidget = new QWidget;
    commentWidget->setObjectName(QString("commentArea_%1").arg(momentId));
    QVBoxLayout* commentLay = new QVBoxLayout(commentWidget);
    commentLay->setSpacing(4);
    commentLay->setContentsMargins(10, 5, 10, 5);

    // 3. 加载评论
    QList<Comment> comments = DBManager::instance().getCommentsByMomentId(momentId);
    if (comments.isEmpty()) {
        QLabel* emptyLab = new QLabel("暂无评论");
        emptyLab->setStyleSheet("color:#999; font-size:12px;");
        commentLay->addWidget(emptyLab);
    } else {
        for (const auto& c : comments) {
            QLabel* cLab = new QLabel(
                QString("[%1] %2：%3")
                    .arg(c.createTime.toString("HH:mm"))
                    .arg(c.userName)
                    .arg(c.content)
                );
            cLab->setStyleSheet("color:#ccc; font-size:12px; padding:2px 0;");
            commentLay->addWidget(cLab);
        }
    }

    // 4. 添加到卡片布局（在footer之后）
    cardLayout->addWidget(commentWidget);
}
