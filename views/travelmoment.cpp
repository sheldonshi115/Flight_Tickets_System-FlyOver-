#include "travelmoment.h"
#include "ui_travelmoment.h"
#include "clickablelabel.h"

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

TravelMoment::TravelMoment(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TravelMoment)
{
    ui->setupUi(this);
    // optional: 初始化显示（若希望有示例内容）
    // MomentItem demo; demo.userName = "示例"; demo.content = "欢迎使用动态模块"; addMoment(demo);
}

TravelMoment::~TravelMoment()
{
    delete ui;
}

// travelmoment.cpp
void TravelMoment::addMoment(const MomentItem &item)
{
    qDebug() << "添加动态：" << item.content;

    // 创建卡片
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

            // 🔍 点击图片预览（后续可扩展）
            imageLabel->setCursor(Qt::PointingHandCursor);
            imageLabel->setProperty("fullImagePath", item.images.first()); // 存储完整路径
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

    // 👇 关键：添加到 momentListLayout（不是 scrollAreaWidgetContents 直接加！）
    ui->momentListLayout->addWidget(card);

    // 刷新滚动区（通常不需要，因为 layout 自动管理）
}

void TravelMoment::refreshList()
{
    // 清空旧的布局项（安全删除）
    QVBoxLayout* lay = new QVBoxLayout(ui->scrollAreaWidgetContents);
    ui->scrollAreaWidgetContents->setLayout(lay);
    lay->setSpacing(10);
    lay->setContentsMargins(0, 0, 0, 0);

    if (lay) {
        QLayoutItem *it;
        while ((it = lay->takeAt(0)) != nullptr) {
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
    }

    // 空状态提示
    if (moments.isEmpty()) {
        QLabel *empty = new QLabel("还没有动态，快来发布第一条吧～");
        empty->setAlignment(Qt::AlignCenter);
        empty->setStyleSheet("color:#999; padding:40px 0;");
        lay->addWidget(empty);
        lay->addStretch();
        return;
    }

    // 逐条创建卡片
    for (const auto &m : moments) {
        // 外层卡片
        QWidget *card = new QWidget;
        card->setObjectName("momentCard");
        // card 背景 + 圆角 + 内边距
        card->setStyleSheet(
            "#momentCard { background: #ffffff; border-radius: 10px; }"
            );
        QVBoxLayout *cardLay = new QVBoxLayout(card);
        cardLay->setContentsMargins(14,14,14,14);
        cardLay->setSpacing(10);

        // 头部：头像、名称、时间
        QHBoxLayout *header = new QHBoxLayout;
        header->setSpacing(10);

        // 头像（圆形灰色占位）
        QLabel *avatar = new QLabel;
        avatar->setFixedSize(44,44);
        avatar->setStyleSheet("background:#e6e6e6; border-radius:22px; color:#888; font-size:12px;");
        avatar->setAlignment(Qt::AlignCenter);
        avatar->setText("头像");

        // 名称 + 时间垂直排列
        QVBoxLayout *nameTime = new QVBoxLayout;
        nameTime->setSpacing(2);
        QLabel *name = new QLabel(m.userName);
        name->setStyleSheet("color:#222; font-weight:600; font-size:14px;");
        QLabel *time = new QLabel(m.publishTime.toString("MM-dd hh:mm"));
        time->setStyleSheet("color:#999; font-size:12px;");
        nameTime->addWidget(name);
        nameTime->addWidget(time);

        header->addWidget(avatar);
        header->addLayout(nameTime);
        header->addStretch();

        cardLay->addLayout(header);

        // 正文（确保深色文字可见）
        QLabel *content = new QLabel;
        content->setWordWrap(true);
        content->setText(m.content.isEmpty() ? QString() : m.content);
        content->setStyleSheet("color:#222; font-size:14px; line-height:1.4;");
        // 让内容与卡片保持一定最小高度（空文本时不要完全塌陷）
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
                    img->setStyleSheet("background:#f0f0f0; border:1px solid #eee;");
                    img->setText("图片");
                    img->setAlignment(Qt::AlignCenter);
                }
                grid->addWidget(img, i/3, i%3);
            }
            cardLay->addLayout(grid);
        }

        // 底部操作栏（点赞/评论）
        QFrame *sep = new QFrame;
        sep->setFrameShape(QFrame::HLine);
        sep->setFrameShadow(QFrame::Sunken);
        sep->setStyleSheet("color:#eee;");
        cardLay->addWidget(sep);

        QHBoxLayout *footer = new QHBoxLayout;
        footer->setSpacing(16);

        // like 按钮风格
        QString likeText = (m.liked ? QString("♥ ") : QString("♡ ")) + QString::number(m.likeCount);
        QPushButton *likeBtn = new QPushButton(likeText);
        likeBtn->setFlat(true);
        likeBtn->setCursor(Qt::PointingHandCursor);
        likeBtn->setStyleSheet("QPushButton{color:#ff4d4f; font-size:13px;} QPushButton:pressed{opacity:0.8;}");

        QPushButton *commentBtn = new QPushButton(QString("评论 ") + QString::number(m.commentCount));
        commentBtn->setFlat(true);
        commentBtn->setCursor(Qt::PointingHandCursor);
        commentBtn->setStyleSheet("QPushButton{color:#333; font-size:13px;}");

        footer->addStretch();
        footer->addWidget(likeBtn);
        footer->addWidget(commentBtn);

        cardLay->addLayout(footer);

        // 连接 lambda（保持和数据同步）
        connect(likeBtn, &QPushButton::clicked, this, [this, id = m.id, likeBtn]() {
            for (auto &mm : moments) {
                if (mm.id == id) {
                    mm.liked = !mm.liked;
                    mm.likeCount += mm.liked ? 1 : -1;
                    QString txt = (mm.liked ? QString("♥ ") : QString("♡ ")) + QString::number(mm.likeCount);
                    likeBtn->setText(txt);
                    break;
                }
            }
        });

        connect(commentBtn, &QPushButton::clicked, this, [this, id = m.id]() {
            QMessageBox::information(this, "评论", "点击了评论，动态ID：" + QString::number(id));
        });

        // 添加卡片到列表
        lay->addWidget(card);
    }

    // 底部拉伸
    if (lay) lay->addStretch();
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
    newItem.userName = "当前用户"; // TODO: 替换为真实用户名

    moments.append(newItem);
    addMoment(newItem);

    // 清空
    ui->contentEdit->clear();
    selectedImages.clear();
    ui->imagePreview->setText("已选择 0 张图片");
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
    // 目前示例只弹出提示框；以后可弹出评论窗口并把评论写入 DB
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

    // 👉 使用 ClickableLabel 而不是 QLabel
    ClickableLabel *preview = new ClickableLabel();
    preview->setPixmap(pixmap.scaled(600, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    preview->setWindowTitle("图片预览 - 点击关闭");
    preview->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    preview->setStyleSheet("background: black;");
    preview->setAlignment(Qt::AlignCenter);
    preview->setAttribute(Qt::WA_DeleteOnClose);

    // ✅ 连接点击信号
    connect(preview, &ClickableLabel::clicked, preview, &QWidget::close);

    preview->show();
}
