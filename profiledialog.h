#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ProfileDialog; // 类名从 UserInfoDialog 改为 ProfileDialog
}

class ProfileDialog : public QDialog // 类名修改
{
    Q_OBJECT

public:
    // 构造函数：接收当前登录账号
    explicit ProfileDialog(const QString& account, QWidget *parent = nullptr);
    ~ProfileDialog();

private slots:
    // 更换头像（槽函数名保持语义一致）
    void on_btnChangeAvatar_clicked();
    // 保存信息（槽函数名保持语义一致）
    void on_btnSave_clicked();
    // 取消操作（新增，符合常见软件交互）
    void on_btnCancel_clicked();

private:
    Ui::ProfileDialog *ui; // UI 指针类名同步修改
    QString m_account;       // 当前登录账号（不可修改）
    QString m_avatarPath;    // 头像文件路径

    // 辅助函数：加载并显示头像
    void loadAndDisplayAvatar();
};

#endif // PROFILEDIALOG_H
