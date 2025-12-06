#include "ProfileRefreshDialog.h"
#include "dbmanager.h"
#include <QStandardPaths>
#include <QDir>

ProfileRefreshDialog::ProfileRefreshDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ProfileRefreshDialog) {
    ui->setupUi(this);

    // 设置默认头像占位符
    currentAvatar = QPixmap(":/images/default_avatar.png");
    avatarFilePath = ""; // 初始化为空

    // 账号通常不可编辑，这里假设你的UI里 lblUsername 是用来显示账号的
    // 如果是 LineEdit 且不可编辑，请设为 setReadOnly(true)
}

ProfileRefreshDialog::~ProfileRefreshDialog() {
    delete ui;
}

void ProfileRefreshDialog::loadProfileData(const UserProfile& profile) {
    // 1. 保存并在界面显示账号 (假设 UI 里 accountLayout 下有个 lblUsername)
    accountStr = profile.account;
    
    qDebug() << "loadProfileData: account=" << profile.account 
             << "nickname=" << profile.nickname 
             << "phone=" << profile.phone 
             << "email=" << profile.email;
    
    ui->lblUsername->setText(profile.account);

    // 2. 填充输入框
    ui->txtNickname->setText(profile.nickname);
    ui->txtPhone->setText(profile.phone);
    ui->txtEmail->setText(profile.email);

    // 3. 设置性别单选框
    if (profile.gender == Gender::Male) ui->rdoMale->setChecked(true);
    else if (profile.gender == Gender::Female) ui->rdoFemale->setChecked(true);
    else ui->rdoUnknown->setChecked(true);

    // 4. 显示头像 (缩放到 Label 大小)
    if (!profile.avatar.isNull()) {
        currentAvatar = profile.avatar;
        // 保持纵横比缩放
        ui->lblAvatar->setPixmap(currentAvatar.scaled(ui->lblAvatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        qDebug() << "头像已加载";
    } else {
        // 如果没有头像，使用默认头像
        currentAvatar = QPixmap(":/images/default_avatar.png");
        if (!currentAvatar.isNull()) {
            ui->lblAvatar->setPixmap(currentAvatar.scaled(ui->lblAvatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            qDebug() << "使用默认头像";
        } else {
            qWarning() << "默认头像加载失败";
        }
    }
}

UserProfile ProfileRefreshDialog::getProfileData() const {
    UserProfile data;
    data.account = accountStr;
    data.nickname = ui->txtNickname->text().trimmed();
    data.phone = ui->txtPhone->text().trimmed();
    data.email = ui->txtEmail->text().trimmed();
    data.avatar = currentAvatar;

    if (ui->rdoMale->isChecked()) data.gender = Gender::Male;
    else if (ui->rdoFemale->isChecked()) data.gender = Gender::Female;
    else data.gender = Gender::Unknown;

    return data;
}

void ProfileRefreshDialog::on_btnChangeAvatar_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择头像", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        if (currentAvatar.load(fileName)) {
            avatarFilePath = fileName; // 保存选择的文件路径
            // 简单展示，实际项目中可能需要做圆形裁剪
            ui->lblAvatar->setPixmap(currentAvatar.scaled(ui->lblAvatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            qDebug() << "头像已选择:" << fileName;
        } else {
            QMessageBox::warning(this, "错误", "无法加载选择的图片文件");
        }
    }
}

void ProfileRefreshDialog::on_btnSave_clicked() {
    // 简单校验
    if (ui->txtNickname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "昵称不能为空！");
        return;
    }
    
    // 确保账号不为空（关键检查）
    if (accountStr.isEmpty()) {
        QMessageBox::critical(this, "错误", "账号信息丢失，无法保存！请重新登录。");
        qWarning() << "严重错误：accountStr 为空，无法保存用户信息";
        return;
    }
    
    // TODO: 这里可以加手机号、邮箱的正则校验

    // 获取编辑后的用户信息
    UserProfile newData = getProfileData();
    
    qDebug() << "开始保存用户信息: account=" << newData.account 
             << "nickname=" << newData.nickname 
             << "phone=" << newData.phone 
             << "email=" << newData.email
             << "accountStr=" << accountStr;
    
    // 处理头像保存
    if (!currentAvatar.isNull()) {
        // 创建头像保存目录 (应用程序数据目录)
        QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString avatarDir = appDataPath + "/avatars";
        QDir dir;
        if (!dir.exists(avatarDir)) {
            dir.mkpath(avatarDir);
        }
        
        // 生成头像文件名（用账号名作为基础）
        QString targetAvatarFile = avatarDir + "/" + newData.account + ".png";
        
        qDebug() << "目标头像文件路径:" << targetAvatarFile;
        
        // 只有当用户选择了新头像时，才需要重新保存
        // 否则仅更新数据库中的图像路径引用
        if (!avatarFilePath.isEmpty()) {
            // 用户选择了新头像，保存到本地文件
            if (currentAvatar.save(targetAvatarFile, "PNG")) {
                qDebug() << "头像已保存到:" << targetAvatarFile;
            } else {
                QMessageBox::warning(this, "警告", "头像保存失败，但其他信息将继续保存");
            }
        }
        
        // 无论是否是新选择的头像，都设置为加载后的版本，确保后续能正确显示
        newData.avatar = currentAvatar;
    }
    
    // 保存到数据库
    bool success = DBManager::instance().saveUserProfile(newData);
    
    if (success) {
        QMessageBox::information(this, "成功", "个人信息保存成功！");
        this->accept(); // 关闭窗口并返回 QDialog::Accepted
    } else {
        QMessageBox::critical(this, "错误", "保存失败：" + DBManager::instance().lastError());
    }
}

void ProfileRefreshDialog::on_btnCancel_clicked() {
    this->reject(); // 关闭窗口并返回 QDialog::Rejected
}
