#include "voucherdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

VoucherDialog::VoucherDialog(const QString& userAccount, double originalPrice, QWidget *parent)
    : QDialog(parent)
    , m_userAccount(userAccount)
    , m_originalPrice(originalPrice)
    , m_voucherValue(0.0)
    , m_finalPrice(originalPrice)
{
    setWindowTitle("选择代金券");
    setFixedSize(450, 500);
    setupUI();
    loadVouchers();
}

VoucherDialog::~VoucherDialog()
{
}

void VoucherDialog::setupUI()
{
    setStyleSheet(R"(
        QDialog {
            background-color: #F8FAFC;
        }
        QLabel {
            color: #374151;
        }
        QListWidget {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 8px;
            padding: 5px;
        }
        QListWidget::item {
            padding: 12px;
            border-bottom: 1px solid #F3F4F6;
            border-radius: 6px;
            margin: 2px;
        }
        QListWidget::item:selected {
            background-color: #DBEAFE;
            border: 2px solid #3B82F6;
        }
        QListWidget::item:hover {
            background-color: #F3F4F6;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 标题
    QLabel *titleLabel = new QLabel("🎫 选择代金券", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #1F2937;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 提示
    QLabel *hintLabel = new QLabel("选择一张代金券抵扣票价，或跳过不使用", this);
    hintLabel->setStyleSheet("font-size: 13px; color: #6B7280;");
    hintLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel);

    // 代金券列表
    m_voucherList = new QListWidget(this);
    m_voucherList->setMinimumHeight(200);
    connect(m_voucherList, &QListWidget::itemClicked, this, &VoucherDialog::onVoucherSelected);
    mainLayout->addWidget(m_voucherList);

    // 价格信息区域
    QFrame *priceFrame = new QFrame(this);
    priceFrame->setStyleSheet("QFrame { background-color: #FFFFFF; border-radius: 10px; border: 1px solid #E5E7EB; }");
    QVBoxLayout *priceLayout = new QVBoxLayout(priceFrame);
    priceLayout->setSpacing(10);

    m_originalPriceLabel = new QLabel(QString("原价：¥%1").arg(m_originalPrice, 0, 'f', 2), priceFrame);
    m_originalPriceLabel->setStyleSheet("font-size: 14px; color: #6B7280;");

    m_discountLabel = new QLabel("优惠：¥0.00", priceFrame);
    m_discountLabel->setStyleSheet("font-size: 14px; color: #10B981;");

    m_finalPriceLabel = new QLabel(QString("应付：¥%1").arg(m_finalPrice, 0, 'f', 2), priceFrame);
    m_finalPriceLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #EF4444;");

    priceLayout->addWidget(m_originalPriceLabel);
    priceLayout->addWidget(m_discountLabel);
    priceLayout->addWidget(m_finalPriceLabel);
    mainLayout->addWidget(priceFrame);

    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_skipBtn = new QPushButton("← 返回", this);
    m_skipBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #9CA3AF;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #6B7280;
        }
    )");
    m_skipBtn->setCursor(Qt::PointingHandCursor);
    connect(m_skipBtn, &QPushButton::clicked, this, &VoucherDialog::onSkip);

    m_noVoucherBtn = new QPushButton("不使用代金券", this);
    m_noVoucherBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #F59E0B;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #D97706;
        }
    )");
    m_noVoucherBtn->setCursor(Qt::PointingHandCursor);
    connect(m_noVoucherBtn, &QPushButton::clicked, this, &VoucherDialog::onNoVoucher);

    m_confirmBtn = new QPushButton("使用选中代金券", this);
    m_confirmBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3B82F6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2563EB;
        }
        QPushButton:disabled {
            background-color: #D1D5DB;
        }
    )");
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setEnabled(false);
    connect(m_confirmBtn, &QPushButton::clicked, this, &VoucherDialog::onConfirm);

    btnLayout->addWidget(m_skipBtn);
    btnLayout->addWidget(m_noVoucherBtn);
    btnLayout->addWidget(m_confirmBtn);
    mainLayout->addLayout(btnLayout);
}

void VoucherDialog::loadVouchers()
{
    m_voucherList->clear();
    m_vouchers = MemberSystem::instance().getAvailableVouchers(m_userAccount);

    if (m_vouchers.isEmpty()) {
        QListWidgetItem *emptyItem = new QListWidgetItem("暂无可用代金券");
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
        emptyItem->setForeground(QColor("#9CA3AF"));
        emptyItem->setTextAlignment(Qt::AlignCenter);
        m_voucherList->addItem(emptyItem);
        return;
    }

    for (int i = 0; i < m_vouchers.size(); ++i) {
        const Voucher& v = m_vouchers[i];
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, i);  // 存储索引
        
        // 根据面值生成友好名称
        QString voucherName = QString("%1元代金券").arg(static_cast<int>(v.value));
        QString expireStr = v.expireDate.toString("yyyy-MM-dd");
        
        QString text = QString("🎁 %1\n   有效期至：%2")
            .arg(voucherName)
            .arg(expireStr);
        item->setText(text);
        item->setSizeHint(QSize(0, 60));  // 设置列表项高度，确保显示完整
        
        // 如果代金券面值大于票价，显示提示
        if (v.value >= m_originalPrice) {
            item->setForeground(QColor("#10B981"));  // 绿色表示可以全额抵扣
        }
        
        m_voucherList->addItem(item);
    }
}

void VoucherDialog::onVoucherSelected(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;

    int index = item->data(Qt::UserRole).toInt();
    if (index < 0 || index >= m_vouchers.size()) return;

    const Voucher& v = m_vouchers[index];
    m_selectedVoucherId = v.id;
    m_selectedVoucherCode = v.code;
    m_voucherValue = v.value;
    m_finalPrice = qMax(0.0, m_originalPrice - m_voucherValue);

    m_discountLabel->setText(QString("优惠：-¥%1").arg(m_voucherValue, 0, 'f', 2));
    m_finalPriceLabel->setText(QString("应付：¥%1").arg(m_finalPrice, 0, 'f', 2));

    m_confirmBtn->setEnabled(true);
}

void VoucherDialog::onConfirm()
{
    accept();
}

void VoucherDialog::onSkip()
{
    reject();  // 返回上一步
}

void VoucherDialog::onNoVoucher()
{
    m_selectedVoucherId.clear();
    m_selectedVoucherCode.clear();
    m_voucherValue = 0.0;
    m_finalPrice = m_originalPrice;
    accept();  // 不使用代金券继续
}
