#include "profiledialog.h"
#include "ui_profiledialog.h"
#include "dbmanager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QRegularExpression> // 统一使用 Qt 6 推荐的正则类

// 工具函数：将头像转为圆形（优化抗锯齿和透明背景处理）
QPixmap circleAvatar(const QPixmap& pixmap, int size) {
    if (pixmap.isNull()) return QPixmap(); // 空图片防护

    // 缩放图片并保持宽高比，使用高质量平滑转换
    QPixmap avatar = pixmap.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)
                         .copy((avatar.width() - size) / 2, (avatar.height() - size) / 2, size, size);

    QPixmap mask(size, size);
    mask.fill(Qt::transparent);

    QPainter painter(&mask);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); // 双重抗锯齿
    painter.setBrush(Qt::white);
    painter.drawEllipse(0, 0, size, size); // 绘制圆形遮罩

    // 给头像添加轻微边框，提升视觉效果
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter resultPainter(&result);
    resultPainter.setRenderHints(QPainter::Antialiasing);
    avatar.setMask(mask.mask()); // 先设置遮罩
    resultPainter.drawPixmap(0, 0, avatar);
    resultPainter.setPen(QPen(QColor(220, 220, 220), 1)); // 灰色细边框
    resultPainter.drawEllipse(0.5, 0.5, size - 1, size - 1); // 边框居中绘制

    return result;
}

// 工具函数：保存头像到本地安全目录（优化文件处理逻辑）
QString saveAvatarToLocal(const QString& originalPath, const QString& account) {
    // 空路径防护
    if (originalPath.isEmpty() || !QFile::exists(originalPath)) {
        qWarning() << "头像文件不存在或路径为空：" << originalPath;
        return "";
    }

    // 头像保存路径：用户文档/FlightTicketSystem/avatars（统一存储，避免冗余）
    QString appDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/FlightTicketSystem/avatars";
    QDir dir(appDir);
    if (!dir.exists() && !dir.mkpath(appDir)) { // 创建目录失败防护
        qWarning() << "创建头像存储目录失败：" << appDir;
        return "";
    }

    // 生成唯一文件名：账号_时间戳.后缀（避免覆盖旧头像）
    QString suffix = originalPath.split(".").last().toLower();
    // 限制支持的图片格式，避免无效文件
    QStringList validSuffixes = {"png", "jpg", "jpeg", "bmp"};
    if (!validSuffixes.contains(suffix)) {
        suffix = "png"; // 强制转换为常见图片格式
    }
    QString timeStamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"); // 毫秒级时间戳，确保唯一
    QString fileName = QString("%1_%2.%3").arg(account).arg(timeStamp).arg(suffix);
    QString savePath = appDir + "/" + fileName;

    // 复制新头像到本地目录（覆盖旧头像，清理冗余）
    QFile oldAvatarFiles(appDir + "/" + account + "_*.png");
    oldAvatarFiles.remove();
    QFile::remove(appDir + "/" + account + "_*.jpg");
    QFile::remove(appDir + "/" + account + "_*.jpeg");
    QFile::remove(appDir + "/" + account + "_*.bmp");

    // 复制文件并验证是否成功
    if (!QFile::copy(originalPath, savePath) || !QFile::exists(savePath)) {
        qWarning() << "头像保存失败：" << originalPath << "→" << savePath;
        return "";
    }

    return savePath;
}

// 构造函数：初始化界面和数据
ProfileDialog::ProfileDialog(const QString& account, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProfileDialog),
    m_account(account), // 初始化登录账号
    m_avatarPath("")    // 初始化头像路径为空
{
    ui->setupUi(this);
    setWindowTitle("个人资料");
    setFixedSize(400, 500); // 固定窗口大小，避免拉伸变形
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 移除帮助按钮

    // 1. 初始化账号显示（不可修改，从登录账号获取）
    ui->lblAccount->setText(account);
    ui->lblAccount->setStyleSheet(R"(
        color: #666;
        font-size: 14px;
        background-color: #f9f9f9;
        padding: 6px 12px;
        border-radius: 4px;
        border: 1px solid #eee;
        min-width: 200px;
    )");
    ui->lblAccount->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 2. 从数据库加载用户现有资料（昵称+头像）
    QVariantMap userProfile = DBManager::instance().getUserProfile(account);
    QString nickname = userProfile["nickname"].toString().trimmed();
    m_avatarPath = userProfile["avatar_path"].toString().trimmed();

    // 初始化昵称输入框（为空时显示占位提示，而非固定文本）
    ui->txtNickname->setPlaceholderText("请输入昵称（1-16个字符）");
    if (!nickname.isEmpty()) {
        ui->txtNickname->setText(nickname);
    }
    ui->txtNickname->setStyleSheet(R"(
        padding: 6px 12px;
        border: 1px solid #ddd;
        border-radius: 4px;
        font-size: 14px;
    )");
    ui->txtNickname->setMaxLength(16); // 限制最大长度，与验证逻辑一致

    // 3. 显示圆形头像（优先级：数据库存储的头像 → 默认头像 → 灰色占位）
    loadAndDisplayAvatar();

    // 4. 美化按钮（贴近常见软件样式，hover效果优化）
    ui->btnChangeAvatar->setStyleSheet(R"(
        QPushButton {
            padding: 6px 12px;
            background: #f5f5f5;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 13px;
        }
        QPushButton:hover {
            background: #eee;
            border-color: #ccc;
        }
        QPushButton:pressed {
            background: #e0e0e0;
        }
    )");
    ui->btnSave->setStyleSheet(R"(
        QPushButton {
            padding: 6px 24px;
            background: #2196F3;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 13px;
            min-width: 100px;
        }
        QPushButton:hover {
            background: #1976D2;
        }
        QPushButton:pressed {
            background: #1565C0;
        }
        QPushButton:disabled {
            background: #90CAF9;
        }
    )");
    ui->btnCancel->setStyleSheet(R"(
        QPushButton {
            padding: 6px 24px;
            background: #f5f5f5;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 13px;
            min-width: 100px;
        }
        QPushButton:hover {
            background: #eee;
            border-color: #ccc;
        }
        QPushButton:pressed {
            background: #e0e0e0;
        }
    )");

    // 5. 标签样式统一
    ui->lblAccountLabel->setStyleSheet("font-size: 14px; color: #333; min-width: 60px; text-align: right;");
    ui->lblNicknameLabel->setStyleSheet("font-size: 14px; color: #333; min-width: 60px; text-align: right;");
}

ProfileDialog::~ProfileDialog()
{
    delete ui;
}

// 辅助函数：加载并显示头像（提取重复逻辑）
void ProfileDialog::loadAndDisplayAvatar() {
    QPixmap avatar;
    // 优先级1：数据库存储的头像
    if (!m_avatarPath.isEmpty() && QFile::exists(m_avatarPath) && avatar.load(m_avatarPath)) {
        ui->lblAvatar->setPixmap(circleAvatar(avatar, 140));
    }
    // 优先级2：默认头像（资源文件）
    else if (avatar.load(":/icons/default_avatar.png")) {
        ui->lblAvatar->setPixmap(circleAvatar(avatar, 140));
    }
    // 优先级3：灰色圆形占位
    else {
        ui->lblAvatar->setStyleSheet(R"(
            background-color: #f0f0f0;
            border-radius: 70px;
            border: 1px solid #eee;
        )");
        ui->lblAvatar->setText("");
    }
    ui->lblAvatar->setAlignment(Qt::AlignCenter);
}

// 更换头像按钮点击事件
void ProfileDialog::on_btnChangeAvatar_clicked()
{
    // 打开文件选择器，限制图片格式
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择头像", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        "图片文件 (*.png *.jpg *.jpeg *.bmp);;所有文件 (*.*)");

    if (!filePath.isEmpty()) {
        // 保存头像到本地统一目录，并更新头像路径
        QString newAvatarPath = saveAvatarToLocal(filePath, m_account);
        if (!newAvatarPath.isEmpty()) {
            m_avatarPath = newAvatarPath;
            // 实时显示新头像
            QPixmap avatar(filePath);
            ui->lblAvatar->setPixmap(circleAvatar(avatar, 140));
        } else {
            QMessageBox::warning(this, "提示", "头像保存失败，请重试！");
        }
    }
}

// 保存按钮点击事件
void ProfileDialog::on_btnSave_clicked()
{
    QString nickname = ui->txtNickname->text().trimmed();

    // 输入验证（符合常见软件规范，顺序优化）
    // 1. 非空验证
    if (nickname.isEmpty()) {
        QMessageBox::warning(this, "提示", "昵称不能为空！");
        ui->txtNickname->setFocus();
        return;
    }
    // 2. 长度验证
    if (nickname.length() > 16) {
        QMessageBox::warning(this, "提示", "昵称长度不能超过16个字符！");
        ui->txtNickname->setFocus();
        return;
    }
    // 3. 特殊字符验证（统一使用 QRegularExpression，兼容 Qt 5/6）
    QRegularExpression specialCharRegex(R"([@#$%^&*()<>{}|\\/:\"'；：”’、])");
    if (specialCharRegex.match(nickname).hasMatch()) {
        QMessageBox::warning(this, "提示", "昵称不能包含 @#$%^&*()<>{}|\\/:\"'；：”’、 等特殊字符！");
        ui->txtNickname->setFocus();
        return;
    }

    // 调用 DBManager 更新到 MySQL 数据库
    if (DBManager::instance().updateUserProfile(m_account, nickname, m_avatarPath)) {
        QMessageBox::information(this, "成功", "个人资料更新成功！");
        accept(); // 关闭窗口并返回成功状态
    } else {
        QMessageBox::critical(this, "失败", "资料保存失败，请检查数据库连接或重试！");
    }
}

// 取消按钮点击事件
void ProfileDialog::on_btnCancel_clicked()
{
    // 询问是否放弃修改
    if (QMessageBox::question(this, "确认", "是否放弃当前修改？",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        reject(); // 关闭窗口并返回取消状态
    }
}
