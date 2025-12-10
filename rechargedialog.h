#ifndef RECHARGEDIALOG_H
#define RECHARGEDIALOG_H

#include <QDialog>

class QLineEdit;
class QButtonGroup;
class QAbstractButton;

class RechargeDialog : public QDialog {
    Q_OBJECT
public:
    explicit RechargeDialog(const QString &account, QWidget *parent = nullptr);
    ~RechargeDialog() override;

signals:
    void rechargeSucceeded(double amount);

private slots:
    void presetClicked(int id);
    void presetButtonClicked(QAbstractButton *btn);
    void presetToggled(QAbstractButton *btn, bool checked);
    void confirmClicked();

private:
    QString m_account;
    QLineEdit *m_customAmountEdit;
    QButtonGroup *m_presetGroup;
    QButtonGroup *m_payGroup;
};

#endif // RECHARGEDIALOG_H
