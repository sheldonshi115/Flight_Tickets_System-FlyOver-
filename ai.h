#ifndef AI_H
#define AI_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QNetworkReply>

class ChatDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum ChatRole {
        SenderRole = Qt::UserRole + 1,
        ContentRole,
        OpacityRole = Qt::UserRole + 3
    };
    explicit ChatDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

namespace Ui {
class AIQueryWidget;
}

class AIQueryWidget : public QWidget {
    Q_OBJECT
public:
    explicit AIQueryWidget(QWidget *parent = nullptr);
    ~AIQueryWidget();

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
};

#endif // AI_H
