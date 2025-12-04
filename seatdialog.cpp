#include "seatdialog.h"
#include "ui_seatdialog.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QStyle>
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression> // 用于正确提取排号

SeatDialog::SeatDialog(const FlightInfo& flightInfo, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeatDialog)
    , m_flightInfo(flightInfo)
{
    ui->setupUi(this);
    initFlightInfoLabel();
    createSeatLayout();
}

SeatDialog::~SeatDialog()
{
    delete ui;
}

void SeatDialog::initFlightInfoLabel()
{
    QString info = QString("航班 %1：%2 → %3（%4）")
                       .arg(m_flightInfo.flightNumber)
                       .arg(m_flightInfo.departureCity)
                       .arg(m_flightInfo.arrivalCity)
                       .arg(m_flightInfo.dateTime);
    ui->lblFlightInfo->setText(info);
}

void SeatDialog::createSeatLayout()
{
    // 初始化头等舱和经济舱网格布局
    QGridLayout* firstClassGrid = new QGridLayout();
    firstClassGrid->setSpacing(10); // 增加间距，避免拥挤
    firstClassGrid->setAlignment(Qt::AlignCenter);
    ui->firstClassLayout->addLayout(firstClassGrid);

    QGridLayout* economyClassGrid = new QGridLayout();
    economyClassGrid->setSpacing(8);
    economyClassGrid->setAlignment(Qt::AlignCenter);
    ui->economyClassLayout->addLayout(economyClassGrid);

    // 提取所有座位的最大排号，用于更新舱位标题
    int maxFirstRow = 0, maxEcoRow = 0;

    for (const auto& seat : m_flightInfo.allSeats) {
        QPushButton* seatBtn = new QPushButton(seat.seatId, this);
        seatBtn->setObjectName("seat_" + seat.seatId); // 唯一名称，方便样式控制
        seatBtn->setProperty("seatId", seat.seatId);
        seatBtn->setProperty("seatState",
                             (seat.state == Available) ? "available" :
                                 (seat.state == Sold) ? "sold" : "selected");
        seatBtn->setProperty("isWindow", seat.isWindow);

        // 关键修复1：正确提取排号（支持多位数排号，如"10A"）
        QRegularExpression rowRegex("(\\d+)"); // 匹配数字部分
        QRegularExpressionMatch match = rowRegex.match(seat.seatId);
        int row = match.hasMatch() ? match.captured(1).toInt() : 0;

        // 关键修复2：正确提取列号（A=0, B=1, ..., F=5）
        QChar colChar = seat.seatId.back();
        int col = colChar.toLatin1() - 'A';

        // 头等舱（1-2排）座位更大
        bool isFirst = (row <= 2);
        seatBtn->setFixedSize(isFirst ? 70 : 60, isFirst ? 50 : 40);

        // 中间过道：C/D之间留1列空隙（列索引2和3之间）
        if (col >= 3) col += 1;

        // 关键修复3：确保可选座位可点击并绑定事件
        updateButtonStyle(seatBtn, seat.state);
        if (seat.state == Available) {
            seatBtn->setEnabled(true); // 强制启用可选座位
            connect(seatBtn, &QPushButton::clicked, this, &SeatDialog::handleSeatClicked);
        }

        // 更新最大排号
        if (isFirst) {
            maxFirstRow = qMax(maxFirstRow, row);
            firstClassGrid->addWidget(seatBtn, row, col);
        } else {
            maxEcoRow = qMax(maxEcoRow, row);
            economyClassGrid->addWidget(seatBtn, row, col);
        }
    }

    // 更新舱位标题（显示实际排号范围）
    ui->firstClassLabel->setText(QString("头等舱 (1-%1排)").arg(maxFirstRow));
    ui->economyClassLabel->setText(QString("经济舱 (%1-%2排)").arg(3).arg(maxEcoRow));
}

void SeatDialog::updateButtonStyle(QPushButton* btn, SeatState state)
{
    switch (state) {
    case Available:
        btn->setEnabled(true);
        btn->setStyleSheet(R"(
            background-color: #4CAF50; /* 亮绿背景 */
            color: #1B5E20; /* 深绿文字（高对比度） */
            border-radius: 6px;
            font-weight: bold;
            border: 2px solid #388E3C;
        )");
        break;
    case Sold:
        btn->setEnabled(false);
        btn->setText("已售"); // 显示"已售"而非"X"，更直观
        btn->setStyleSheet(R"(
            background-color: #E0E0E0; /* 浅灰背景 */
            color: #757575; /* 深灰文字 */
            border-radius: 6px;
            border: 1px solid #BDBDBD;
        )");
        break;
    case Selected:
        btn->setEnabled(false);
        btn->setStyleSheet(R"(
            background-color: #FFC107; /* 亮黄背景 */
            color: #E65100; /* 橙棕文字（高对比度） */
            border-radius: 6px;
            font-weight: bold;
            border: 2px solid #FFA000;
        )");
        break;
    }
}

void SeatDialog::handleSeatClicked()
{
    QPushButton* clickedBtn = qobject_cast<QPushButton*>(sender());
    if (!clickedBtn) return;

    // 取消上一个选中座位
    if (m_selectedSeat && m_selectedSeat != clickedBtn) {
        updateButtonStyle(m_selectedSeat, Available); // 恢复可选样式
        m_selectedSeat->setEnabled(true); // 重新启用
    }

    // 选中当前座位
    m_selectedSeat = clickedBtn;
    updateButtonStyle(m_selectedSeat, Selected); // 设置选中样式

    // 关键修复4：选中座位后启用确认按钮
    ui->btnConfirm->setEnabled(true);
}

void SeatDialog::on_btnConfirm_clicked()
{
    if (m_selectedSeat) {
        accept();
    } else {
        QMessageBox::warning(this, "提示", "请选择座位");
    }
}

QString SeatDialog::getSelectedSeatId() const
{
    return m_selectedSeat ? m_selectedSeat->property("seatId").toString() : "";
}
