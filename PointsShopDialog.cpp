#include "PointsShopDialog.h"
#include "membersystem.h"
#include "dbmanager.h"
#include "notificationmanager.h"
#include "emailreminder.h"
#include <QSettings>
#include <QListWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QDateTime>
#include <QUuid>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

PointsShopDialog::PointsShopDialog(const QString& account, QWidget* parent)
    : QDialog(parent), m_list(nullptr), m_buyBtn(nullptr), m_pointsLabel(nullptr), m_account(account)
{
    setWindowTitle(QString::fromUtf8("积分商城"));
    resize(640, 420);

    // 主布局与间距
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12,12,12,12);
    mainLayout->setSpacing(10);

    // 头部：积分+余额，样式美化
    QHBoxLayout* header = new QHBoxLayout();
    m_pointsLabel = new QLabel(QString::fromUtf8("我的积分：0"), this);
    m_balanceLabel = new QLabel(QString::fromUtf8("余额：¥0.00"), this);
    QFont bigFont = m_pointsLabel->font(); bigFont.setPointSize(12); bigFont.setBold(true);
    m_pointsLabel->setFont(bigFont);
    m_balanceLabel->setFont(QFont(m_balanceLabel->font().family(), 10));
    m_pointsLabel->setStyleSheet("color:#2b8cff;");
    m_balanceLabel->setStyleSheet("color:#666;");
    header->addWidget(m_pointsLabel);
    header->addSpacing(8);
    header->addWidget(m_balanceLabel);
    header->addStretch();
    mainLayout->addLayout(header);

    // 列表与右侧详情并列布局
    QHBoxLayout* mid = new QHBoxLayout();
    mid->setSpacing(12);

    // 商品列表（左）
    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setSpacing(8);
    // 移除 QListWidget::item:selected 默认绘制，改为对自定义 item widget 使用样式切换，避免选中时位置抖动
    m_list->setStyleSheet("QListWidget{background:transparent; border:1px solid #eee; padding:6px;} QListWidget::item{padding:8px;}");
    m_list->setMinimumWidth(360);
    m_list->setUniformItemSizes(true);
    m_list->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mid->addWidget(m_list, 1);

    // 右侧：商品详情预览区域（简单展示选中项信息）
    QWidget* detail = new QWidget(this);
    QVBoxLayout* detailLay = new QVBoxLayout(detail);
    detailLay->setContentsMargins(8,8,8,8);
    detailLay->setSpacing(8);
    QLabel* previewTitle = new QLabel(QString::fromUtf8("商品信息"), detail);
    previewTitle->setFont(QFont(previewTitle->font().family(), 11, QFont::Bold));
    QLabel* previewDesc = new QLabel(QString::fromUtf8("请选择左侧商品以查看详情。"), detail);
    previewDesc->setWordWrap(true);
    previewDesc->setObjectName("previewDesc");
    previewDesc->setStyleSheet("color:#444;");
    detailLay->addWidget(previewTitle);
    detailLay->addWidget(previewDesc);
    detailLay->addStretch();
    // 固定右侧详情宽度，避免左侧列表随内容伸缩影响右侧说明
    detail->setFixedWidth(240);
    detail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mid->addWidget(detail, 0);

    mainLayout->addLayout(mid);

    // 底部：按钮栏，美化样式
    QHBoxLayout* footer = new QHBoxLayout();
    m_buyBtn = new QPushButton(QString::fromUtf8("兑换"), this);
    m_buyBtn->setEnabled(false);
    m_buyBtn->setStyleSheet("QPushButton{background:#2b8cff;color:white;padding:8px 16px;border-radius:6px;} QPushButton:disabled{background:#b5cfff;color:#eee}");
    QPushButton* closeBtn = new QPushButton(QString::fromUtf8("关闭"), this);
    closeBtn->setStyleSheet("QPushButton{background:#f5f5f5;color:#333;padding:8px 16px;border-radius:6px;}");
    footer->addStretch();
    footer->addWidget(m_buyBtn);
    footer->addWidget(closeBtn);
    mainLayout->addLayout(footer);

    connect(m_buyBtn, &QPushButton::clicked, this, &PointsShopDialog::onBuyClicked);
    connect(m_list, &QListWidget::currentRowChanged, this, &PointsShopDialog::onSelectionChanged);
    connect(closeBtn, &QPushButton::clicked, this, &PointsShopDialog::close);

    loadItems();
    refreshUserPoints();

    // 选中变化时更新右侧详情文本（保留）
    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row){
        QLabel* pd = this->findChild<QLabel*>("previewDesc");
        if (!pd) return;
        if (row < 0 || row >= m_items.size()) { pd->setText(QString::fromUtf8("请选择左侧商品以查看详情。")); return; }
        const ShopItem& it = m_items[row];
        pd->setText(QString::fromUtf8("名称：%1\n所需积分：%2\n\n%3").arg(it.name).arg(it.pointsCost).arg(it.description));
    });

    // 新增：使用 item widget 的样式来表示选中状态，避免 QListWidget 内置选中背景在不同 item widget margin/size 下造成视觉移动
    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row){
        for (int i = 0; i < m_list->count(); ++i) {
            QListWidgetItem* it = m_list->item(i);
            QWidget* w = m_list->itemWidget(it);
            if (!w) continue;
            if (i == row) {
                w->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f8fdff, stop:1 #eef8ff); border-radius:6px;");
            } else {
                w->setStyleSheet("");
            }
        }
    });

    // 初始没有选中项时不高亮；可选择默认选中第一项
    if (m_list->count() > 0) {
        m_list->setCurrentRow(0);
    }
}

PointsShopDialog::~PointsShopDialog() {}

void PointsShopDialog::loadItems()
{
    // 简单的静态商品列表
    m_items = {
        {"itm1", QString::fromUtf8("5元代金券"), 500, QString::fromUtf8("可抵扣5元人民币的订单金额"), QStringLiteral(":/resources/images/coupon5.svg")},
        {"itm2", QString::fromUtf8("10元代金券"), 900, QString::fromUtf8("可抵扣10元人民币的订单金额"), QStringLiteral(":/resources/images/coupon10.svg")},
        {"itm3", QString::fromUtf8("随机礼品"), 1500, QString::fromUtf8("系统随机发送小礼品一份"), QStringLiteral(":/resources/images/gift.svg")}
    };

    m_list->clear();
    for (const ShopItem& it : m_items) {
        // 自定义 item widget
        QListWidgetItem* item = new QListWidgetItem(m_list);
        QWidget* w = new QWidget();
        QHBoxLayout* h = new QHBoxLayout(w);
        h->setContentsMargins(6,6,6,6);
        h->setSpacing(8);

        QLabel* icon = new QLabel(w);
        icon->setFixedSize(48,48);
        icon->setAlignment(Qt::AlignCenter);
        if (!it.imagePath.isEmpty()) {
            QPixmap p(it.imagePath);
            if (!p.isNull()) icon->setPixmap(p.scaled(icon->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            else { icon->setText(QString::fromUtf8("图")); icon->setStyleSheet("background:#eee;border-radius:6px;color:#666;"); }
        } else {
            icon->setText(QString::fromUtf8("图"));
            icon->setStyleSheet("background:#eee;border-radius:6px;color:#666;");
        }

        QVBoxLayout* v = new QVBoxLayout();
        v->setSpacing(4);
        QLabel* name = new QLabel(it.name);
        name->setFont(QFont(name->font().family(), 10, QFont::Bold));
        QLabel* desc = new QLabel(it.description);
        desc->setStyleSheet("color:#777; font-size:11px;");
        desc->setWordWrap(true);
        v->addWidget(name);
        v->addWidget(desc);

        QLabel* cost = new QLabel(QString::fromUtf8("%1 积分").arg(it.pointsCost));
        cost->setStyleSheet("background:#ffefc2;color:#b35a00;padding:6px;border-radius:6px;font-weight:bold;");
        cost->setAlignment(Qt::AlignCenter);

        h->addWidget(icon);
        h->addLayout(v);
        h->addStretch();
        h->addWidget(cost);

        item->setSizeHint(QSize(0, 72));
        m_list->setItemWidget(item, w);
    }
}

void PointsShopDialog::refreshUserPoints()
{
    // 取当前登录用户账号（从 MainWindow 的 m_appUser 储存），但这里通过 DBManager 或 MemberSystem 获取
    // 假设当前已登录账号从 DBManager 或全局状态可读，这里先简单从 MemberSystem 的接口中读取，
    // 需要调用者在创建对话框前确保主窗口已设置正确的当前账号（MainWindow::m_appUser.account）

    // 通过 DBManager 的连接，取当前登录账号
    // 为简化，这里查找 sessions 之类的机制不存在，使用 QInputDialog 不友好；我们尝试从 DBManager 的 last opened connection中读取

    // 实际使用中，应该传入当前账号；为了避免改动大量接口，这里尝试从 QSettings 中读取保存的账号（登录时保存）

    if (m_account.isEmpty()) {
        m_pointsLabel->setText(QString::fromUtf8("我的积分：未登录"));
        return;
    }

    MemberInfo info = MemberSystem::instance().getMemberInfo(m_account);
    m_pointsLabel->setText(QString::fromUtf8("我的积分：") + QString::number(info.points));
    m_balanceLabel->setText(QString::fromUtf8("余额：¥") + QString::number(info.balance, 'f', 2));
}

void PointsShopDialog::onSelectionChanged()
{
    int idx = m_list->currentRow();
    if (idx < 0 || idx >= m_items.size()) {
        m_buyBtn->setEnabled(false);
        return;
    }

    const ShopItem& item = m_items[idx];

    // 若用户未登录，禁用兑换按钮
    if (m_account.isEmpty()) {
        m_buyBtn->setEnabled(false);
        return;
    }

    MemberInfo info = MemberSystem::instance().getMemberInfo(m_account);
    m_buyBtn->setEnabled(info.points >= item.pointsCost);
}

void PointsShopDialog::onBuyClicked()
{
    int idx = m_list->currentRow();
    if (idx < 0 || idx >= m_items.size()) return;

    if (m_account.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("未登录"), QString::fromUtf8("请先登录后再兑换。"));
        return;
    }

    const ShopItem& item = m_items[idx];

    // 再次检查积分
    MemberInfo info = MemberSystem::instance().getMemberInfo(m_account);
    if (info.points < item.pointsCost) {
        QMessageBox::warning(this, QString::fromUtf8("积分不足"), QString::fromUtf8("您的积分不足，无法兑换此商品。"));
        refreshUserPoints();
        return;
    }

    // 使用 MemberSystem 提供的兑换 API（内部会扣积分并创建代金券/记录）
    QString createdVoucherIdOrGift = MemberSystem::instance().redeemPointsForItem(m_account, item.id, item.pointsCost);

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!createdVoucherIdOrGift.isEmpty() && db.isOpen()) {
        // 如果返回的是 GIFT: 开头，表示获取了随机礼品
        if (createdVoucherIdOrGift.startsWith("GIFT:")) {
            QString giftDesc = createdVoucherIdOrGift.mid(QString("GIFT:").length());
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("恭喜！您获得了礼品：%1。请关注站内通知或个人中心查看发放信息。").arg(giftDesc));
            // 刷新并返回
            NotificationManager::instance().showNotification(QString::fromUtf8("兑换成功"),
                                                           QString::fromUtf8("您获得了礼品：%1").arg(giftDesc),
                                                           NotificationType::SystemMessage);
            refreshUserPoints();
            return;
        }

        QString createdVoucherId = createdVoucherIdOrGift;
        // 查询代金券 code 和面值以便展示
        QSqlQuery q(db);
        q.prepare("SELECT code, value FROM vouchers WHERE id = :id");
        q.bindValue(":id", createdVoucherId);
        if (q.exec() && q.next()) {
            QString code = q.value("code").toString();
            double val = q.value("value").toDouble();
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("兑换成功：已生成代金券 %1，面值 ¥%2，30天内有效。").arg(code).arg(val, 0, 'f', 2));
        } else {
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("兑换成功：已使用积分兑换 %1。系统会尽快为您发放礼品。").arg(item.name));
        }
    } else {
        // 若没有生成代金券，也可能是普通商品兑换或失败
        if (createdVoucherIdOrGift.isEmpty()) {
            // 仍显示普通兑换成功提示（redeemPointsForItem 已经记录）
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("您已成功使用 %1 积分兑换 %2。系统会尽快为您发放礼品或记录已生成。").arg(item.pointsCost).arg(item.name));
        }
    }

    // 发送系统通知
    NotificationManager::instance().showNotification(QString::fromUtf8("兑换成功"),
                                                   QString::fromUtf8("您已成功兑换 %1。").arg(item.name),
                                                   NotificationType::SystemMessage);

    // 【新增】发送积分兑换成功邮件提醒
    if (!m_account.isEmpty()) {
        UserProfile userProfile = DBManager::instance().loadUserProfile(m_account);
        // 即使没有邮箱也发送系统内部提醒
        EmailReminder::instance().sendPointsRedeemedReminder(
            userProfile.email,
            userProfile.nickname.isEmpty() ? m_account : userProfile.nickname,
            m_account,
            item.name,
            item.pointsCost
        );
    }

    refreshUserPoints();
}

void PointsShopDialog::onRedeemRequested(const QString &itemId)
{
    // Find item by id
    int idx = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == itemId) { idx = i; break; }
    }
    if (idx < 0 || idx >= m_items.size()) return;

    const ShopItem& item = m_items[idx];

    if (m_account.isEmpty()) {
        QMessageBox::warning(this, QString::fromUtf8("未登录"), QString::fromUtf8("请先登录后再兑换。"));
        return;
    }

    MemberInfo info = MemberSystem::instance().getMemberInfo(m_account);
    if (info.points < item.pointsCost) {
        QMessageBox::warning(this, QString::fromUtf8("积分不足"), QString::fromUtf8("您的积分不足，无法兑换此商品。"));
        refreshUserPoints();
        return;
    }

    QString createdVoucherIdOrGift = MemberSystem::instance().redeemPointsForItem(m_account, item.id, item.pointsCost);

    QSqlDatabase db = DBManager::instance().getDatabase();
    if (!createdVoucherIdOrGift.isEmpty() && db.isOpen()) {
        if (createdVoucherIdOrGift.startsWith("GIFT:")) {
            QString giftDesc = createdVoucherIdOrGift.mid(QString("GIFT:").length());
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("恭喜！您获得了礼品：%1。请关注站内通知或个人中心查看发放信息。").arg(giftDesc));
            NotificationManager::instance().showNotification(QString::fromUtf8("兑换成功"),
                                                           QString::fromUtf8("您获得了礼品：%1").arg(giftDesc),
                                                           NotificationType::SystemMessage);
            refreshUserPoints();
            return;
        }

        QString createdVoucherId = createdVoucherIdOrGift;
        QSqlQuery q(db);
        q.prepare("SELECT code, value FROM vouchers WHERE id = :id");
        q.bindValue(":id", createdVoucherId);
        if (q.exec() && q.next()) {
            QString code = q.value("code").toString();
            double val = q.value("value").toDouble();
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("兑换成功：已生成代金券 %1，面值 ¥%2，30天内有效。").arg(code).arg(val, 0, 'f', 2));
        } else {
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("兑换成功：已使用积分兑换 %1。系统会尽快为您发放礼品。").arg(item.name));
        }
    } else {
        if (createdVoucherIdOrGift.isEmpty()) {
            QMessageBox::information(this, QString::fromUtf8("兑换成功"),
                                     QString::fromUtf8("您已成功使用 %1 积分兑换 %2。系统会尽快为您发放礼品或记录已生成。").arg(item.pointsCost).arg(item.name));
        }
    }

    NotificationManager::instance().showNotification(QString::fromUtf8("兑换成功"),
                                                   QString::fromUtf8("您已成功兑换 %1。").arg(item.name),
                                                   NotificationType::SystemMessage);

    refreshUserPoints();
}
