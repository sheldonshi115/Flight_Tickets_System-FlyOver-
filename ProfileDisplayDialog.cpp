#include "ProfileDisplayDialog.h"
#include "login.h"
#include <QMessageBox>
#include <QMainWindow>
#include <QDebug>

ProfileDisplayDialog::ProfileDisplayDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ProfileDisplayDialog) {
    ui->setupUi(this);
    
    // 优化对话框背景样式 - 纯白色背景
    this->setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #FFFFFF, stop:1 #F8FAFC);
        }
        QLabel {
            color: #475569;
            font-size: 14px;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3B82F6, stop:1 #2563EB);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #60A5FA, stop:1 #3B82F6);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2563EB, stop:1 #1E40AF);
        }
    )");

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
        ui->lblAvatar->setToolTip(QString::fromUtf8("点击修改头像"));
    } else {
        // 如果没有头像，显示提示文字
        ui->lblAvatar->setText(QString::fromUtf8("点击此处\n添加头像"));
        ui->lblAvatar->setAlignment(Qt::AlignCenter);
        ui->lblAvatar->setStyleSheet(
            "background-color: #F0F9FF; "
            "border-radius: 60px; "
            "border: 2px dashed #BFDBFE; "
            "color: #60A5FA; "
            "font-size: 14px; "
            "font-weight: 600;"
        );
        ui->lblAvatar->setToolTip(QString::fromUtf8("点击修改头像"));
    }
}

void ProfileDisplayDialog::on_btnEditProfile_clicked() {
    // 1. 创建编辑窗口
    ProfileRefreshDialog editDlg(this);

    // 2. 将当前数据传给编辑窗口
    editDlg.loadProfileData(m_currentUser);

    // 3. 模态运行窗口
    if (editDlg.exec() == QDialog::Accepted) {
        // 4. 如果用户点了保存，获取新数据并更新本地和界面
        UserProfile newData = editDlg.getProfileData();
        
        // 更新本地的 m_currentUser（保存按钮已在 ProfileRefreshDialog 中调用了 DBManager）
        m_currentUser = newData;
        
        // 更新界面显示
        updateDisplay(newData);
        qDebug() << "[ProfileDisplayDialog] 用户资料已保存，关闭展示对话框返回 Accepted";
        this->accept();
    }
}

void ProfileDisplayDialog::on_btnLogout_clicked() {
    // 确认退出登录
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "退出登录",
        "确定要退出登录吗？",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        qDebug() << "[ProfileDisplayDialog] 用户确认退出登录，准备重启应用程序";
        
        // 关闭个人信息窗口
        this->reject();
        
        // 查找主窗口
        QWidget *mainWindow = nullptr;
        QWidget *current = this->parentWidget();
        while (current) {
            if (qobject_cast<QMainWindow*>(current)) {
                mainWindow = current;
                break;
            }
            current = current->parentWidget();
        }
        
        if (mainWindow) {
            qDebug() << "[ProfileDisplayDialog] 找到主窗口，准备重启";
            
            // 关闭主窗口
            mainWindow->close();
            
            // 退出并重启应用程序
            QApplication::exit(1000);
        }
    }
}
