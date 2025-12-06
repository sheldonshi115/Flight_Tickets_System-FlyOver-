#pragma once
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QButtonGroup>
#include "ui_ProfileRefreshDialog.h" // 假设你的UI文件生成了这个
#include "UserProfile.h"

class ProfileRefreshDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProfileRefreshDialog(QWidget *parent = nullptr);
    ~ProfileRefreshDialog();

    // 加载现有数据到界面
    void loadProfileData(const UserProfile& profile);
    // 获取界面上编辑后的数据
    UserProfile getProfileData() const;

private slots:
    void on_btnChangeAvatar_clicked(); // 更换头像
    void on_btnSave_clicked();         // 保存
    void on_btnCancel_clicked();       // 取消

private:
    Ui::ProfileRefreshDialog *ui;
    QPixmap currentAvatar; // 暂存当前头像
    QString accountStr;    // 账号通常只读，存一下
    QString avatarFilePath; // 新增：记录头像文件路径
};
