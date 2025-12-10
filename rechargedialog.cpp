#include "rechargedialog.h"
#include "membersystem.h"
#include <QAbstractButton>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStyle>

RechargeDialog::RechargeDialog(const QString &account, QWidget *parent)
    : QDialog(parent), m_account(account), m_customAmountEdit(nullptr), m_presetGroup(nullptr)
{
    setWindowTitle(QString::fromUtf8("充值飞机币"));
    setModal(true);
    setMinimumWidth(420);

    QLabel *title = new QLabel(QString::fromUtf8("选择充值金额"), this);
    QFont ft = title->font(); ft.setPointSize(14); ft.setBold(true); title->setFont(ft);

    QString radioFix = R"(
        QRadioButton {
            spacing: 8px;
            color: #0F172A;
        }
        QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #D1D5DB;
            border-radius: 9px;
            background-color: #FFFFFF;
        }
        QRadioButton::indicator:checked {
            background-color: #3B82F6;
            border-color: #3B82F6;
        }
        QRadioButton::indicator:disabled {
            background-color: #F3F4F6;
            border-color: #E5E7EB;
        }
    )";
    // 将本地单选框修复样式应用到此对话框，覆盖全局主题导致的黑色指示器问题
    this->setStyleSheet(this->styleSheet() + radioFix);

    // Preset amounts
    QWidget *presets = new QWidget(this);
    QGridLayout *grid = new QGridLayout(presets);
    grid->setSpacing(10);
    grid->setContentsMargins(0,0,0,0);

    QList<int> amounts = {10, 30, 50, 100, 200, 500};
    m_presetGroup = new QButtonGroup(this);
    m_presetGroup->setExclusive(true);
    int col = 0, row = 0, id = 0;
    for (int a : amounts) {
        QPushButton *b = new QPushButton(QString::fromUtf8("¥%1").arg(a));
        b->setCheckable(true);
        b->setMinimumSize(120, 44);
        b->setStyleSheet("QPushButton{border-radius:8px;background:#f5f7fa;} QPushButton:checked{background:#ffd54f;color:#222}");
        m_presetGroup->addButton(b, id);
        grid->addWidget(b, row, col);
        col++;
        if (col >= 3) { col = 0; row++; }
        id++;
    }

    // Add a "Custom" selectable option as part of presets
    QPushButton *bCustom = new QPushButton(QString::fromUtf8("自定义"));
    bCustom->setCheckable(true);
    bCustom->setMinimumSize(120, 44);
    bCustom->setStyleSheet("QPushButton{border-radius:8px;background:#f5f7fa;} QPushButton:checked{background:#e3f2fd;color:#1976d2}");
    m_presetGroup->addButton(bCustom, id);
    grid->addWidget(bCustom, row, col);
    // increment id (not strictly needed further)
    id++;

    // Use the button pointer signal to reliably resolve id (Qt signal overloading can be tricky)
    connect(m_presetGroup, &QButtonGroup::buttonClicked, this, &RechargeDialog::presetButtonClicked);
    // Also listen for toggles so that selecting (checked=true) reliably enables custom input
    connect(m_presetGroup, &QButtonGroup::buttonToggled, this, &RechargeDialog::presetToggled);

    // Custom amount
    QLabel *customLabel = new QLabel(QString::fromUtf8("自定义金额 (元)："), this);
    m_customAmountEdit = new QLineEdit(this);
    m_customAmountEdit->setPlaceholderText("输入金额，例如 88.00");
    m_customAmountEdit->setFixedWidth(160);
    m_customAmountEdit->setClearButtonEnabled(true);
    // 默认情况下自定义输入不可用，只有选择“自定义”预设后才可编辑
    m_customAmountEdit->setEnabled(false);

    QHBoxLayout *customLay = new QHBoxLayout;
    customLay->addWidget(customLabel);
    customLay->addWidget(m_customAmountEdit);
    customLay->addStretch();

    // Payment methods (visual only)
    QLabel *payLabel = new QLabel(QString::fromUtf8("支付方式："), this);
    QRadioButton *rbWx = new QRadioButton(QString::fromUtf8("微信支付"), this);
    QRadioButton *rbAli = new QRadioButton(QString::fromUtf8("支付宝"), this);
    QRadioButton *rbCard = new QRadioButton(QString::fromUtf8("银行卡"), this);
    rbWx->setChecked(true);
    // 将支付方式放入按钮组以便可以互斥选择（视觉效果）
    m_payGroup = new QButtonGroup(this);
    m_payGroup->addButton(rbWx, 0);
    m_payGroup->addButton(rbAli, 1);
    m_payGroup->addButton(rbCard, 2);
    m_payGroup->setExclusive(true);
    QHBoxLayout *payLay = new QHBoxLayout;
    payLay->addWidget(payLabel);
    payLay->addWidget(rbWx);
    payLay->addWidget(rbAli);
    payLay->addWidget(rbCard);
    payLay->addStretch();

    // Confirm / Cancel
    QPushButton *btnConfirm = new QPushButton(QString::fromUtf8("确认充值"), this);
    btnConfirm->setDefault(true);
    btnConfirm->setStyleSheet("background:#1976d2;color:white;padding:10px 18px;border-radius:8px;");
    QPushButton *btnCancel = new QPushButton(QString::fromUtf8("取消"), this);

    connect(btnConfirm, &QPushButton::clicked, this, &RechargeDialog::confirmClicked);
    connect(btnCancel, &QPushButton::clicked, this, &RechargeDialog::reject);

    QHBoxLayout *btnLay = new QHBoxLayout;
    btnLay->addStretch();
    btnLay->addWidget(btnCancel);
    btnLay->addWidget(btnConfirm);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(title);
    main->addSpacing(6);
    main->addWidget(presets);
    main->addLayout(customLay);
    main->addSpacing(6);
    main->addLayout(payLay);
    main->addStretch();
    main->addLayout(btnLay);

}

RechargeDialog::~RechargeDialog() {}

void RechargeDialog::presetClicked(int id)
{
    // Map id to amounts list
    QList<int> amounts = {10, 30, 50, 100, 200, 500};
    int presetCount = amounts.size();
    if (id >= 0 && id < presetCount) {
        // preset selected: disable custom input and set the amount
        m_customAmountEdit->blockSignals(true);
        m_customAmountEdit->setText(QString::number(amounts[id]));
        m_customAmountEdit->setEnabled(false);
        m_customAmountEdit->blockSignals(false);
    } else if (id == presetCount) {
        // custom selected: enable input and clear for user entry
        m_customAmountEdit->setEnabled(true);
        m_customAmountEdit->clear();
        m_customAmountEdit->setFocus(Qt::OtherFocusReason);
    }
}

void RechargeDialog::presetButtonClicked(QAbstractButton *btn)
{
    int id = m_presetGroup->id(btn);
    presetClicked(id);
}

void RechargeDialog::presetToggled(QAbstractButton *btn, bool checked)
{
    Q_UNUSED(btn);
    // Re-evaluate current checked button and apply behavior (this covers cases where clicked didn't fire)
    QAbstractButton *current = m_presetGroup->checkedButton();
    if (!current) {
        // none checked: disable custom input
        m_customAmountEdit->setEnabled(false);
        return;
    }
    int id = m_presetGroup->id(current);
    QList<int> amounts = {10, 30, 50, 100, 200, 500};
    int presetCount = amounts.size();
    if (id >= 0 && id < presetCount) {
        m_customAmountEdit->blockSignals(true);
        m_customAmountEdit->setText(QString::number(amounts[id]));
        m_customAmountEdit->setEnabled(false);
        m_customAmountEdit->blockSignals(false);
    } else if (id == presetCount) {
        if (checked) {
            m_customAmountEdit->setEnabled(true);
            m_customAmountEdit->clear();
            m_customAmountEdit->setFocus(Qt::OtherFocusReason);
        } else {
            m_customAmountEdit->setEnabled(false);
        }
    }
}

void RechargeDialog::confirmClicked()
{
    bool ok = false;
    double amount = m_customAmountEdit->text().toDouble(&ok);
    if (!ok || amount <= 0.0) {
        QMessageBox::warning(this, QString::fromUtf8("无效金额"), QString::fromUtf8("请输入有效的充值金额（大于0）。"));
        return;
    }

    // Call member system to add balance
    bool success = MemberSystem::instance().addBalance(m_account, amount, QString::fromUtf8("用户充值（UI）"));
    if (success) {
        QMessageBox::information(this, QString::fromUtf8("充值成功"), QString::fromUtf8("充值成功：已为您的账户充值 ¥%1").arg(amount));
        emit rechargeSucceeded(amount);
        accept();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("充值失败"), QString::fromUtf8("充值失败，请稍后重试。"));
    }
}

// Note: custom amount is now a selectable preset; no separate textChanged handler required.
