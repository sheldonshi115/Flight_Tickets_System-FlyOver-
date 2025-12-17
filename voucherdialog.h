#ifndef VOUCHERDIALOG_H
#define VOUCHERDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "membersystem.h"

class VoucherDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VoucherDialog(const QString& userAccount, double originalPrice, QWidget *parent = nullptr);
    ~VoucherDialog();

    QString getSelectedVoucherId() const { return m_selectedVoucherId; }
    QString getSelectedVoucherCode() const { return m_selectedVoucherCode; }
    double getVoucherValue() const { return m_voucherValue; }
    double getFinalPrice() const { return m_finalPrice; }

private slots:
    void onVoucherSelected(QListWidgetItem *item);
    void onConfirm();
    void onSkip();
    void onNoVoucher();

private:
    void setupUI();
    void loadVouchers();

    QString m_userAccount;
    double m_originalPrice;
    double m_voucherValue;
    double m_finalPrice;
    QString m_selectedVoucherId;
    QString m_selectedVoucherCode;

    QListWidget *m_voucherList;
    QLabel *m_originalPriceLabel;
    QLabel *m_discountLabel;
    QLabel *m_finalPriceLabel;
    QPushButton *m_confirmBtn;
    QPushButton *m_skipBtn;
    QPushButton *m_noVoucherBtn;

    QList<Voucher> m_vouchers;
};

#endif // VOUCHERDIALOG_H
