#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QScopedPointer> // 新增：用于智能指针管理
#include <QDateTime>      // 新增：用于登录失败锁定
#include <QTimer>
#include <QVector>
#include <QPointF>
#include <QSettings>      // 新增：用于记住我功能
#include <QCheckBox>      // 新增：记住我复选框
#include <QHideEvent>
#include <QShowEvent>
#include "register.h"

namespace Ui {
class LoginDialog;
}

// 粒子结构体
struct Particle {
    QPointF pos;
    QPointF velocity;
    float alpha;
    float size;
    float alphaSpeed;
};

// 自定义粒子动画面板
class ParticlePanel : public QWidget
{
    Q_OBJECT
public:
    explicit ParticlePanel(QWidget *parent = nullptr);
    ~ParticlePanel();
    
    void startAnimation();  // 启动动画
    void stopAnimation();   // 停止动画
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    
private:
    QVector<Particle> m_particles;
    int m_timerId;
    float m_gradientOffset; // 渐变动画偏移
    
    void initParticles();
    void updateParticles();
};

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    QString get_account()const;

private slots:
    void on_loginButton_clicked();      // 登录按钮
    void on_registerLink_clicked();     // 跳转到注册界面
    void onForgotPasswordClicked();     // 忘记密码链接
    void toggleAdminCodeInput();        // 切换管理员验证码输入显示

private:
    Ui::LoginDialog *ui;
    QScopedPointer<RegisterDialog> registerDialog;
    int m_failedAttempts = 0;
    QDateTime m_lockUntil;
    ParticlePanel *m_particlePanel = nullptr;
    QCheckBox *m_rememberCheck = nullptr;
    QString m_adminCode;
    
    // 记住我功能
    void loadRememberedCredentials();
    void saveCredentials();
    void clearCredentials();
    
protected:
    void hideEvent(QHideEvent *event) override;
    void showEvent(QShowEvent *event) override;
};

#endif // LOGINDIALOG_H
