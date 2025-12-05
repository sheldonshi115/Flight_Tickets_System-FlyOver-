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
    // 强制模态+窗口属性（关键）
    this->setModal(true);
    this->setWindowModality(Qt::ApplicationModal); // 阻塞整个应用
    this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    // 强制调整大小+显示
    this->resize(900, 700);
    this->raise(); // 置顶
    this->activateWindow(); // 激活窗口
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
    QGridLayout* firstClassGrid = new QGridLayout();
    firstClassGrid->setSpacing(10);
    firstClassGrid->setAlignment(Qt::AlignCenter);

    // 安全清空原有布局（核心修复：避免takeAt(0)空指针）
    while (ui->firstClassLayout->count() > 0) {
        QLayoutItem* item = ui->firstClassLayout->takeAt(0);
        if (item->layout()) delete item->layout();
        delete item;
    }
    ui->firstClassLayout->addLayout(firstClassGrid);

    QGridLayout* economyClassGrid = new QGridLayout();
    economyClassGrid->setSpacing(8);
    economyClassGrid->setAlignment(Qt::AlignCenter);

    // 安全清空经济舱布局
    while (ui->economyClassLayout->count() > 0) {
        QLayoutItem* item = ui->economyClassLayout->takeAt(0);
        if (item->layout()) delete item->layout();
        delete item;
    }
    ui->economyClassLayout->addLayout(economyClassGrid);

    int maxFirstRow = 0, maxEcoRow = 0;

    for (const auto& seat : m_flightInfo.allSeats) {
        QPushButton* seatBtn = new QPushButton(seat.seatId, this);
        seatBtn->setObjectName("seat_" + seat.seatId);
        seatBtn->setProperty("seatId", seat.seatId);
        seatBtn->setProperty("seatState",
                             (seat.state == Available) ? "available" :
                                 (seat.state == Sold) ? "sold" : "selected");
        seatBtn->setProperty("isWindow", seat.isWindow);

        // 正确提取排号
        QRegularExpression rowRegex("(\\d+)");
        QRegularExpressionMatch match = rowRegex.match(seat.seatId);
        int row = match.hasMatch() ? match.captured(1).toInt() : 0;

        // 简化列索引计算（避免溢出）
        QChar colChar = seat.seatId.back();
        int col = colChar.toLatin1() - 'A';
        if (col >= 3) col += 1; // 过道空隙
        if (col > 6) col = 6; // 限制最大列数

        bool isFirst = (row <= 2);
        seatBtn->setFixedSize(isFirst ? 70 : 60, isFirst ? 50 : 40);

        updateButtonStyle(seatBtn, seat.state);
        if (seat.state == Available) {
            seatBtn->setEnabled(true);
            connect(seatBtn, &QPushButton::clicked, this, &SeatDialog::handleSeatClicked);
        }

        int gridRow = isFirst ? row : (row - 2);
        if (isFirst) {
            maxFirstRow = qMax(maxFirstRow, row);
            firstClassGrid->addWidget(seatBtn, gridRow, col);
        } else {
            maxEcoRow = qMax(maxEcoRow, row);
            economyClassGrid->addWidget(seatBtn, gridRow, col);
        }
    }

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
        updateButtonStyle(m_selectedSeat, Available);
        m_selectedSeat->setEnabled(true);
    }

    // 选中当前座位
    m_selectedSeat = clickedBtn;
    updateButtonStyle(m_selectedSeat, Selected);

    // 强制启用确认按钮（兜底）
    ui->btnConfirm->setEnabled(true);
    // 可选：给确认按钮加高亮样式
    ui->btnConfirm->setStyleSheet(R"(
        QPushButton {
            background-color: #2196F3;
            color: white;
            border-radius: 6px;
            padding: 8px 20px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
    )");
}
void SeatDialog::on_btnConfirm_clicked()
{
    if (m_selectedSeat) {
        // 核心修复：确认选中值后再关闭对话框
        QString seatId = m_selectedSeat->property("seatId").toString();
        qDebug() << "确认选中座位：" << seatId;
        accept(); // 触发exec()返回Accepted
    } else {
        QMessageBox::warning(this, "提示", "请选择座位");
    }
}

QString SeatDialog::getSelectedSeatId() const
{
    return m_selectedSeat ? m_selectedSeat->property("seatId").toString() : "";
}
