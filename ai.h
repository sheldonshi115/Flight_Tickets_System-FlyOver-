#ifndef AI_H
#define AI_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QNetworkReply>
#include <QPixmap>
#include <QJsonObject>
#include <QJsonArray>

class FlightManager; // 前向声明

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
    // Agent模式接口
    void setFlightManager(FlightManager *manager);
    void setCurrentUser(const QString &account);
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
    void onModeToggleClicked(); // 模式切换槽
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

    // Agent模式相关成员
    bool m_isAgentMode = false; // Agent模式标志
    FlightManager *m_flightManager = nullptr; // FlightManager引用
    QString m_currentUserAccount; // 当前用户账号

    // Agent工作流状态
    enum AgentState {
        Idle,           // 空闲
        WaitingFlight,  // 等待选择航班
        WaitingSeat,    // 等待选择座位
        WaitingConfirm  // 等待确认购票
    };
    AgentState m_agentState = Idle;
    QList<class Flight> m_searchedFlights; // 查询到的航班列表
    int m_selectedFlightIndex = -1; // 选中的航班索引
    QString m_selectedSeat; // 选中的座位号

    // Agent辅助方法
    void processAgentResponse(const QJsonObject &agentData, const QString &answer);
    bool parseFlightQuery(const QString &query, QString &departure, QString &arrival, QString &date);
    void showFlightOptions(const QList<class Flight> &flights);
    void executeBooking();
    bool resolveAgentDateRange(const QString &timeType, const QString &specificDate,
                               QDate &startDate, QDate &endDate, bool &useExactDate) const;
    QList<class Flight> convertAgentFlights(const QJsonArray &flightArray) const;
public:
    // 设置拥有窗口（用于定位目标按钮等），但不作为 parent
    void setOwnerWindow(QWidget *owner) { m_ownerWindow = owner; }
};

#endif // AI_H
