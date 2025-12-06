#pragma once
#include <QDialog>
#include "ui_ProfileDisplayDialog.h"
#include "UserProfile.h"
#include "ProfileRefreshDialog.h"

class ProfileDisplayDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProfileDisplayDialog(QWidget *parent = nullptr);
    ~ProfileDisplayDialog();

    // 初始化/更新界面显示
    void updateDisplay(const UserProfile& profile);
    UserProfile getCurrentProfile() const { return m_currentUser; }
private slots:
    void on_btnEditProfile_clicked(); // 点击编辑资料

private:
    Ui::ProfileDisplayDialog *ui;
    UserProfile m_currentUser; // 保存当前用户数据
};
