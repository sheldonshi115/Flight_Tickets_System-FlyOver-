#include "pointitemwidget.h"
#include "PointsShopDialog.h" // for ShopItem struct
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QFile>

PointItemWidget::PointItemWidget(const ShopItem &item, QWidget *parent)
    : QWidget(parent), m_itemId(item.id)
{
    m_title = new QLabel(item.name, this);
    QFont titleFont = m_title->font(); titleFont.setPointSize(11); titleFont.setBold(true); m_title->setFont(titleFont);
    m_title->setStyleSheet("color: #333333;");
    m_title->setWordWrap(true);
    m_cost = new QLabel(QString::fromUtf8("%1 积分").arg(item.pointsCost), this);
    m_cost->setStyleSheet("color: #666666;");
    m_cost->setWordWrap(false);
    m_redeem = new QPushButton(QString::fromUtf8("兑换"), this);
    m_redeem->setFixedHeight(32);
    m_redeem->setStyleSheet("background:#3B82F6;color:white;border-radius:6px;padding:4px 12px;");

    QVBoxLayout *v = new QVBoxLayout;
    v->setSpacing(10);
    v->setContentsMargins(0,8,0,8);
    v->addWidget(m_title);
    // show description line if any (keeps layout similar to previous list)
    QLabel *desc = new QLabel(item.description, this);
    QFont descFont = desc->font(); descFont.setPointSize(9); desc->setFont(descFont);
    desc->setStyleSheet("color: #888888; line-height: 1.4;");
    desc->setWordWrap(true);
    desc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    desc->setMinimumHeight(50);
    v->addWidget(desc);
    v->addWidget(m_cost);
    v->addStretch();
    // action row to align button to right-bottom
    QHBoxLayout *actionRow = new QHBoxLayout;
    actionRow->addStretch();
    actionRow->addWidget(m_redeem);
    v->addLayout(actionRow);

    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(12,8,12,8);
    h->setSpacing(8);
    // Text-only card: add spacing on left to simulate thumbnail area
    h->addSpacing(6);
    h->addLayout(v);
    setLayout(h);
    setStyleSheet(R"(
        background: #ffffff;
        border-radius:8px;
        border: 1px solid rgba(0,0,0,0.08);
        padding:8px;
    )");
    setMinimumWidth(260);
    setMinimumHeight(160);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // ensure background fill so text palette applies correctly
    setAutoFillBackground(true);
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, QColor("#ffffff"));
    pal.setColor(QPalette::WindowText, QColor("#333333"));
    this->setPalette(pal);

    // make sure title/desc have minimum heights to avoid half-rendering
    QFontMetrics fmTitle(titleFont);
    m_title->setMinimumHeight(fmTitle.height() + 8);
    QFontMetrics fmDesc(descFont);
    desc->setMinimumHeight(fmDesc.lineSpacing() + 6);

    connect(m_redeem, &QPushButton::clicked, this, [this]() { emit redeemRequested(m_itemId); });
}

void PointItemWidget::setAffordable(bool ok)
{
    m_redeem->setEnabled(ok);
    if (ok) m_redeem->setStyleSheet("background:#1976d2;color:white;border-radius:6px;padding:4px 8px;");
    else m_redeem->setStyleSheet("background:#e0e0e0;color:#888;border-radius:6px;padding:4px 8px;");
}

QSize PointItemWidget::sizeHint() const
{
    return QSize(240, 140);
}

QSize PointItemWidget::minimumSizeHint() const
{
    return QSize(220, 120);
}
