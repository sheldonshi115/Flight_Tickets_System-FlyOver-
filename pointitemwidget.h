#ifndef POINTITEMWIDGET_H
#define POINTITEMWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;

struct ShopItem;

class PointItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit PointItemWidget(const ShopItem& item, QWidget *parent = nullptr);
    void setAffordable(bool ok);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void redeemRequested(const QString &itemId);

private:
    QString m_itemId;
    QLabel *m_title;
    QLabel *m_cost;
    QPushButton *m_redeem;
};

#endif // POINTITEMWIDGET_H
