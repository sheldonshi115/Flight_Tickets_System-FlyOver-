#pragma once

#include <QDialog>
#include <QList>
#include <QString>

struct ShopItem {
    QString id;
    QString name;
    int pointsCost;
    QString description;
    QString imagePath; // resource path, e.g. :/resources/coupon50.svg
};

class QListWidget;
class QPushButton;
class QLabel;

class PointsShopDialog : public QDialog {
    Q_OBJECT
public:
    explicit PointsShopDialog(const QString& account, QWidget* parent = nullptr);
    ~PointsShopDialog();

private slots:
    void onBuyClicked();
    void onSelectionChanged();
    void onRedeemRequested(const QString &itemId);

private:
    void loadItems();
    void refreshUserPoints();
    void refreshUserVouchers();

    QList<ShopItem> m_items;
    QListWidget* m_list;
    QListWidget* m_voucherList;
    QPushButton* m_buyBtn;
    QLabel* m_pointsLabel;
    QLabel* m_balanceLabel;
    QString m_account;
};
