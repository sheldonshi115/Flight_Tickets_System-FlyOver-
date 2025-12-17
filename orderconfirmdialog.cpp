#include "orderconfirmdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGridLayout>

OrderConfirmDialog::OrderConfirmDialog(const OrderInfo& info, QWidget *parent)
    : QDialog(parent)
    , m_info(info)
{
    setWindowTitle("订单确认");
    setFixedSize(500, 550);
    setupUI();
}

OrderConfirmDialog::~OrderConfirmDialog()
{
}

void OrderConfirmDialog::setupUI()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #F8FAFC;
        }
        QLabel {
            color: #374151;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // 标题
    QLabel *titleLabel = new QLabel("✈️ 订单确认", this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #1F2937;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 航班信息卡片
    QFrame *flightCard = new QFrame(this);
    flightCard->setStyleSheet(R"(
        QFrame {
            background-color: #FFFFFF;
            border-radius: 12px;
            border: 1px solid #E5E7EB;
        }
    )");
    QVBoxLayout *flightLayout = new QVBoxLayout(flightCard);
    flightLayout->setSpacing(12);
    flightLayout->setContentsMargins(20, 20, 20, 20);

    // 航班号
    QLabel *flightNoLabel = new QLabel(QString("航班号：%1").arg(m_info.flightNumber), flightCard);
    flightNoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #3B82F6;");
    flightLayout->addWidget(flightNoLabel);

    // 路线
    QHBoxLayout *routeLayout = new QHBoxLayout();
    QLabel *depLabel = new QLabel(m_info.departureCity, flightCard);
    depLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1F2937;");
    QLabel *arrowLabel = new QLabel("  ✈️ →  ", flightCard);
    arrowLabel->setStyleSheet("font-size: 16px; color: #6B7280;");
    QLabel *arrLabel = new QLabel(m_info.arrivalCity, flightCard);
    arrLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1F2937;");
    routeLayout->addWidget(depLabel);
    routeLayout->addWidget(arrowLabel);
    routeLayout->addWidget(arrLabel);
    routeLayout->addStretch();
    flightLayout->addLayout(routeLayout);

    // 出发时间
    QLabel *timeLabel = new QLabel(QString("🕐 出发时间：%1").arg(m_info.departureTime), flightCard);
    timeLabel->setStyleSheet("font-size: 14px; color: #6B7280;");
    flightLayout->addWidget(timeLabel);

    // 座位号
    QLabel *seatLabel = new QLabel(QString("💺 座位号：%1").arg(m_info.seatNumber), flightCard);
    seatLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #10B981;");
    flightLayout->addWidget(seatLabel);

    mainLayout->addWidget(flightCard);

    // 价格信息卡片
    QFrame *priceCard = new QFrame(this);
    priceCard->setStyleSheet(R"(
        QFrame {
            background-color: #FFFFFF;
            border-radius: 12px;
            border: 1px solid #E5E7EB;
        }
    )");
    QVBoxLayout *priceLayout = new QVBoxLayout(priceCard);
    priceLayout->setSpacing(10);
    priceLayout->setContentsMargins(20, 15, 20, 15);

    // 原价
    QHBoxLayout *origRow = new QHBoxLayout();
    QLabel *origLabel = new QLabel("票价原价", priceCard);
    origLabel->setStyleSheet("font-size: 14px; color: #6B7280;");
    QLabel *origValue = new QLabel(QString("¥%1").arg(m_info.originalPrice, 0, 'f', 2), priceCard);
    origValue->setStyleSheet("font-size: 14px; color: #6B7280;");
    origRow->addWidget(origLabel);
    origRow->addStretch();
    origRow->addWidget(origValue);
    priceLayout->addLayout(origRow);

    // 代金券优惠
    if (m_info.voucherValue > 0) {
        QHBoxLayout *voucherRow = new QHBoxLayout();
        QLabel *voucherLabel = new QLabel(QString("代金券 (%1)").arg(m_info.voucherCode), priceCard);
        voucherLabel->setStyleSheet("font-size: 14px; color: #10B981;");
        QLabel *voucherValue = new QLabel(QString("-¥%1").arg(m_info.voucherValue, 0, 'f', 2), priceCard);
        voucherValue->setStyleSheet("font-size: 14px; color: #10B981;");
        voucherRow->addWidget(voucherLabel);
        voucherRow->addStretch();
        voucherRow->addWidget(voucherValue);
        priceLayout->addLayout(voucherRow);
    }

    // 分割线
    QFrame *line = new QFrame(priceCard);
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("background-color: #E5E7EB;");
    line->setFixedHeight(1);
    priceLayout->addWidget(line);

    // 应付金额
    QHBoxLayout *finalRow = new QHBoxLayout();
    QLabel *finalLabel = new QLabel("应付金额", priceCard);
    finalLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #1F2937;");
    QLabel *finalValue = new QLabel(QString("¥%1").arg(m_info.finalPrice, 0, 'f', 2), priceCard);
    finalValue->setStyleSheet("font-size: 22px; font-weight: bold; color: #EF4444;");
    finalRow->addWidget(finalLabel);
    finalRow->addStretch();
    finalRow->addWidget(finalValue);
    priceLayout->addLayout(finalRow);

    mainLayout->addWidget(priceCard);

    // 余额信息
    QLabel *balanceLabel = new QLabel(QString("💰 当前飞机币余额：¥%1").arg(m_info.userBalance, 0, 'f', 2), this);
    if (m_info.userBalance >= m_info.finalPrice) {
        balanceLabel->setStyleSheet("font-size: 14px; color: #10B981;");
    } else {
        balanceLabel->setStyleSheet("font-size: 14px; color: #EF4444;");
    }
    balanceLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(balanceLabel);

    // 余额不足提示
    if (m_info.userBalance < m_info.finalPrice) {
        QLabel *warningLabel = new QLabel("⚠️ 余额不足，请先充值！", this);
        warningLabel->setStyleSheet("font-size: 14px; color: #EF4444; font-weight: bold;");
        warningLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(warningLabel);
    }

    mainLayout->addStretch();

    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);

    QPushButton *cancelBtn = new QPushButton("← 返回上一步", this);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #9CA3AF;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 14px 30px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #6B7280;
        }
    )");
    cancelBtn->setCursor(Qt::PointingHandCursor);
    connect(cancelBtn, &QPushButton::clicked, this, &OrderConfirmDialog::onCancel);

    QPushButton *confirmBtn = new QPushButton("确认支付", this);
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #10B981;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 14px 40px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #059669;
        }
        QPushButton:disabled {
            background-color: #D1D5DB;
        }
    )");
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setEnabled(m_info.userBalance >= m_info.finalPrice);
    connect(confirmBtn, &QPushButton::clicked, this, &OrderConfirmDialog::onConfirm);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(confirmBtn);
    mainLayout->addLayout(btnLayout);
}

void OrderConfirmDialog::onConfirm()
{
    accept();
}

void OrderConfirmDialog::onCancel()
{
    reject();
}
