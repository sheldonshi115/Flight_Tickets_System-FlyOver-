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
    // 主题美化
    setStyleSheet(R"(
        QDialog {
            background-color: #F9FAFB; /* 更轻的背景色 */
            font-family: "Microsoft YaHei", sans-serif;
        }
        QLabel {
            color: #374151; /* 深灰色文字 */
        }
        QListWidget {
            background-color: #FFFFFF;
            border: none;
            border-radius: 0;
            padding: 0px 0px 4px 0px;
            outline: none;
        }
        QListWidget::item {
            padding: 12px 12px;
            border: none;
            margin: 0;
            color: #1F2937;
        }
        QListWidget::item:selected {
            background-color: #EBF5FF; /* 更柔和的选中色 */
            color: #1E40AF;
            border-left: 4px solid #3B82F6; /* 左侧边框高亮 */
        }
        QListWidget::item:hover {
            background-color: #F3F4F6;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(18); // 调整主布局间距
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // 标题
    QLabel *titleLabel = new QLabel("✨ 选择代金券", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: 700; color: #111827;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 提示
    QLabel *hintLabel = new QLabel("选择一张代金券抵扣票价，或跳过不使用", this);
    hintLabel->setStyleSheet("font-size: 14px; color: #6B7280;");
    hintLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel);

    // 代金券列表
    m_voucherList = new QListWidget(this);
    m_voucherList->setMinimumHeight(230);
    m_voucherList->setSpacing(6);
    m_voucherList->setWordWrap(true);
    m_voucherList->setUniformItemSizes(false);
    connect(m_voucherList, &QListWidget::itemClicked, this, &VoucherDialog::onVoucherSelected);
    mainLayout->addWidget(m_voucherList);

    // 价格信息区域
    QFrame *priceFrame = new QFrame(this);
    priceFrame->setObjectName("priceFrame");
    priceFrame->setStyleSheet(R"(
        #priceFrame { 
            background-color: #FFFFFF; 
            border-radius: 0px; 
            border: none; 
            padding: 16px;
        }
    )");
    QGridLayout *priceLayout = new QGridLayout(priceFrame);
    priceLayout->setSpacing(12);
    priceLayout->setContentsMargins(10, 10, 10, 10);

    m_originalPriceLabel = new QLabel("原价：", priceFrame);
    m_originalPriceLabel->setStyleSheet("font-size: 15px; color: #6B7280;");
    QLabel* originalPriceValue = new QLabel(QString("¥%1").arg(m_originalPrice, 0, 'f', 2), priceFrame);
    originalPriceValue->setStyleSheet("font-size: 15px; color: #6B7280; font-weight: 500;");
    originalPriceValue->setAlignment(Qt::AlignRight);

    m_discountLabel = new QLabel("优惠：", priceFrame);
    m_discountLabel->setStyleSheet("font-size: 15px; color: #10B981;");
    QLabel* discountValue = new QLabel("¥0.00", priceFrame);
    discountValue->setObjectName("discountValueLabel");
    discountValue->setStyleSheet("font-size: 15px; color: #10B981; font-weight: 500;");
    discountValue->setAlignment(Qt::AlignRight);

    m_finalPriceLabel = new QLabel("应付：", priceFrame);
    m_finalPriceLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #EF4444;");
    QLabel* finalPriceValue = new QLabel(QString("¥%1").arg(m_finalPrice, 0, 'f', 2), priceFrame);
    finalPriceValue->setObjectName("finalPriceValueLabel");
    finalPriceValue->setStyleSheet("font-size: 22px; font-weight: bold; color: #EF4444;");
    finalPriceValue->setAlignment(Qt::AlignRight);

    priceLayout->addWidget(m_originalPriceLabel, 0, 0);
    priceLayout->addWidget(originalPriceValue, 0, 1);
    priceLayout->addWidget(m_discountLabel, 1, 0);
    priceLayout->addWidget(discountValue, 1, 1);
    priceLayout->addWidget(m_finalPriceLabel, 2, 0);
    priceLayout->addWidget(finalPriceValue, 2, 1);
    mainLayout->addWidget(priceFrame);

    // 按钮区域
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);

    m_skipBtn = new QPushButton("← 返回", this);
    m_skipBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #FFFFFF;
            color: #4B5563;
            border: 1px solid #D1D5DB;
            border-radius: 10px;
            padding: 12px 20px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #F9FAFB;
            border-color: #9CA3AF;
        }
    )");
    m_skipBtn->setCursor(Qt::PointingHandCursor);
    connect(m_skipBtn, &QPushButton::clicked, this, &VoucherDialog::onSkip);

    m_noVoucherBtn = new QPushButton("不使用", this);
    m_noVoucherBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #FBBF24;
            color: #422006;
            border: none;
            border-radius: 10px;
            padding: 12px 20px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #F59E0B;
        }
    )");
    m_noVoucherBtn->setCursor(Qt::PointingHandCursor);
    connect(m_noVoucherBtn, &QPushButton::clicked, this, &VoucherDialog::onNoVoucher);

    m_confirmBtn = new QPushButton("使用代金券", this);
    m_confirmBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3B82F6;
            color: white;
            border: none;
            border-radius: 10px;
            padding: 12px 20px;
            font-size: 15px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #2563EB;
        }
        QPushButton:disabled {
            background-color: #D1D5DB;
            color: #9CA3AF;
        }
    )");
    m_confirmBtn->setCursor(Qt::PointingHandCursor);
    m_confirmBtn->setEnabled(false);
    connect(m_confirmBtn, &QPushButton::clicked, this, &VoucherDialog::onConfirm);

    btnLayout->addWidget(m_skipBtn);
    btnLayout->addWidget(m_noVoucherBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_confirmBtn);
    mainLayout->addLayout(btnLayout);
}

void VoucherDialog::loadVouchers()
{
    m_voucherList->clear();
    m_vouchers = MemberSystem::instance().getAvailableVouchers(m_userAccount);

    if (m_vouchers.isEmpty()) {
        QListWidgetItem *emptyItem = new QListWidgetItem("🙁 暂无可用代金券");
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
        emptyItem->setForeground(QColor("#9CA3AF"));
        emptyItem->setTextAlignment(Qt::AlignCenter);
        m_voucherList->addItem(emptyItem);
        return;
    }

    for (int i = 0; i < m_vouchers.size(); ++i) {
        const Voucher& v = m_vouchers[i];
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, i);

        // 卡片式展示：金额 + 名称 + 到期日
        QWidget *card = new QWidget();
        QHBoxLayout *h = new QHBoxLayout(card);
        h->setContentsMargins(10, 8, 10, 8);
        h->setSpacing(10);

        QLabel *amount = new QLabel(QString("¥%1").arg(static_cast<int>(v.value)), card);
        amount->setStyleSheet("font-size: 22px; font-weight: 800; color: #0EA5E9;");
        amount->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QVBoxLayout *info = new QVBoxLayout();
        info->setSpacing(4);
        QLabel *name = new QLabel(QString("%1元代金券").arg(static_cast<int>(v.value)), card);
        name->setStyleSheet("font-size: 15px; font-weight: 700; color: #1F2937;");
        QLabel *expire = new QLabel(QString("有效期至 %1").arg(v.expireDate.toString("yyyy-MM-dd")), card);
        expire->setStyleSheet("font-size: 12px; color: #6B7280;");
        info->addWidget(name);
        info->addWidget(expire);

        h->addWidget(amount);
        h->addLayout(info);
        h->addStretch();

        item->setSizeHint(QSize(0, 68));
        m_voucherList->addItem(item);
        m_voucherList->setItemWidget(item, card);
    }
}

void VoucherDialog::onVoucherSelected(QListWidgetItem *item)
{
    if (!item || !item->data(Qt::UserRole).isValid()) {
        m_confirmBtn->setEnabled(false);
        return;
    }

    int index = item->data(Qt::UserRole).toInt();
    if (index < 0 || index >= m_vouchers.size()) return;

    const Voucher& v = m_vouchers[index];
    m_selectedVoucherId = v.id;
    m_selectedVoucherCode = v.code;
    m_voucherValue = v.value;
    m_finalPrice = m_originalPrice - m_voucherValue;
    if (m_finalPrice < 0) m_finalPrice = 0;

    findChild<QLabel*>("discountValueLabel")->setText(QString("¥%1").arg(m_voucherValue, 0, 'f', 2));
    findChild<QLabel*>("finalPriceValueLabel")->setText(QString("¥%1").arg(m_finalPrice, 0, 'f', 2));

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
