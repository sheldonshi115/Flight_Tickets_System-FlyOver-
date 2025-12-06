#include "ProfileRefreshDialog.h"

ProfileRefreshDialog::ProfileRefreshDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ProfileRefreshDialog) {
    ui->setupUi(this);

    // 设置默认头像占位符
    currentAvatar = QPixmap(":/images/default_avatar.png");

    // 账号通常不可编辑，这里假设你的UI里 lblUsername 是用来显示账号的
    // 如果是 LineEdit 且不可编辑，请设为 setReadOnly(true)
}

ProfileRefreshDialog::~ProfileRefreshDialog() {
    delete ui;
}

void ProfileRefreshDialog::loadProfileData(const UserProfile& profile) {
    // 1. 保存并在界面显示账号 (假设 UI 里 accountLayout 下有个 lblUsername)
    accountStr = profile.account;
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
        currentAvatar.load(fileName);
        // 简单展示，实际项目中可能需要做圆形裁剪
        ui->lblAvatar->setPixmap(currentAvatar.scaled(ui->lblAvatar->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ProfileRefreshDialog::on_btnSave_clicked() {
    // 简单校验
    if (ui->txtNickname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "昵称不能为空！");
        return;
    }
    // TODO: 这里可以加手机号、邮箱的正则校验

    this->accept(); // 关闭窗口并返回 QDialog::Accepted
}

void ProfileRefreshDialog::on_btnCancel_clicked() {
    this->reject(); // 关闭窗口并返回 QDialog::Rejected
}
