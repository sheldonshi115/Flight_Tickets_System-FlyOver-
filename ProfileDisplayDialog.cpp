#include "ProfileDisplayDialog.h"
ProfileDisplayDialog::ProfileDisplayDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ProfileDisplayDialog) {
    ui->setupUi(this);

    // 初始显示
    updateDisplay(m_currentUser);
}

ProfileDisplayDialog::~ProfileDisplayDialog() {
    delete ui;
}

void ProfileDisplayDialog::updateDisplay(const UserProfile& profile) {
    m_currentUser = profile; // 更新内存中的数据

    // 根据 UI 对象树的命名设置文本
    ui->lblUsername->setText(profile.account); // 账号
    ui->lblNickname->setText(profile.nickname);
    ui->lblPhone->setText(profile.phone);
    ui->lblEmail->setText(profile.email);

    // 性别转字符串
    QString genderStr;
    switch(profile.gender) {
    case Gender::Male: genderStr = "男"; break;
    case Gender::Female: genderStr = "女"; break;
    default: genderStr = "未知"; break;
    }
    // 假设你显示性别的 Label 叫 lblGender (截图中没完全展开，我推测叫这个)
    // 如果是 lblGenderLabel 只是标题，那应该有个对应的显示值的 Label
    // 假设叫 lblGenderValue (请根据实际情况修改)
    ui->lblGender->setText(genderStr);


    // 设置头像
    if (!profile.avatar.isNull()) {
        ui->lblAvatar->setPixmap(getRoundPixmap(profile.avatar, ui->lblAvatar->size()));
    }
}

void ProfileDisplayDialog::on_btnEditProfile_clicked() {
    // 1. 创建编辑窗口
    ProfileRefreshDialog editDlg(this);

    // 2. 将当前数据传给编辑窗口
    editDlg.loadProfileData(m_currentUser);

    // 3. 模态运行窗口
    if (editDlg.exec() == QDialog::Accepted) {
        // 4. 如果用户点了保存，获取新数据并更新展示
        UserProfile newData = editDlg.getProfileData();

        // 此处通常需要调用 API 或数据库保存数据
        // bool saveSuccess = MyDatabase::save(newData);

        // 更新界面
        updateDisplay(newData);
    }
}
