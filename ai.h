#ifndef AI_H
#define AI_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QNetworkReply>
#include <QPixmap>

class ChatDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum ChatRole {
        SenderRole = Qt::UserRole + 1,
        ContentRole,
        OpacityRole = Qt::UserRole + 3
    };
    explicit ChatDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void setUserAvatar(const QPixmap &pixmap) { m_userAvatar = pixmap; }
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QPixmap m_userAvatar; // 来自主窗口或登录后的用户头像
};

namespace Ui {
class AIQueryWidget;
}

class AIQueryWidget : public QWidget {
    Q_OBJECT
public:
    explicit AIQueryWidget(QWidget *parent = nullptr);
    ~AIQueryWidget();
    // 将用户头像传入AI对话组件，用于在用户消息前显示头像
    void setUserAvatar(const QPixmap &pixmap);
    // 动画接口：带动画显示窗口（类似Linux的动态弹出）
    void showWithAnimation();
    // 触发带动画关闭（类似macOS的淡出/缩小删除）
    void animateClose();
    // （保留系统标题栏拖动，移除自定义拖拽）
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event)override;

private slots:
    void on_queryButton_clicked();
    void onReplyFinished(QNetworkReply *reply);
    void onReturnPressed();
    void onRequestTimeout();
    void cancelCurrentRequest();
    void updateItemOpacity(QListWidgetItem *item);

    // 新增：UI安全操作辅助槽函数（强制主线程执行）
    void safeAddChatItem(const QString &sender, const QString &content);
    void safeRemoveLoadingItem();
    void safeScrollToBottom();
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::AIQueryWidget *ui=nullptr;
    QNetworkAccessManager *m_netManager=nullptr;
    QString m_apiUrl = "http://localhost:8000/query_flight";
    ChatDelegate *m_chatDelegate= nullptr;
    QListWidgetItem *m_loadingItem = nullptr; // 显式初始化空指针
    bool m_isRequesting = false;
    QNetworkReply *m_currentReply = nullptr;
    QTimer *m_requestTimer = nullptr;
    bool m_isClosing = false; // 标记是否正在执行关闭动画
    QWidget *m_ownerWindow = nullptr; // 用于动画定位到主窗口的控件
public:
    // 设置拥有窗口（用于定位目标按钮等），但不作为 parent
    void setOwnerWindow(QWidget *owner) { m_ownerWindow = owner; }
};

#endif // AI_H
