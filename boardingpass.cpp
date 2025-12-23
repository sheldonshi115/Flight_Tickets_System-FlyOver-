#include "boardingpass.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QTextDocument>
#include <QDir>

QString BoardingPass::generateBoardingPassHTML(const PassengerInfo& passenger, 
                                                 const FlightInfo& flight)
{
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: "Microsoft YaHei", Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 30px;
        }
        .boarding-pass {
            width: 800px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #3B82F6 0%, #2563EB 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 {
            font-size: 36px;
            margin-bottom: 10px;
            letter-spacing: 2px;
        }
        .header .airline {
            font-size: 18px;
            opacity: 0.9;
        }
        .content {
            padding: 40px;
        }
        .section {
            margin-bottom: 30px;
        }
        .section-title {
            color: #3B82F6;
            font-size: 14px;
            font-weight: bold;
            text-transform: uppercase;
            margin-bottom: 10px;
            letter-spacing: 1px;
        }
        .info-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        .info-item {
            padding: 15px;
            background: #F0F9FF;
            border-radius: 10px;
            border-left: 4px solid #3B82F6;
        }
        .info-label {
            font-size: 12px;
            color: #64748B;
            margin-bottom: 5px;
        }
        .info-value {
            font-size: 18px;
            color: #1E40AF;
            font-weight: bold;
        }
        .flight-route {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 30px;
            background: linear-gradient(135deg, #DBEAFE 0%, #BFDBFE 100%);
            border-radius: 15px;
            margin: 20px 0;
        }
        .city {
            text-align: center;
            flex: 1;
        }
        .city-code {
            font-size: 48px;
            font-weight: bold;
            color: #1E40AF;
        }
        .city-name {
            font-size: 16px;
            color: #475569;
            margin-top: 5px;
        }
        .plane-icon {
            font-size: 36px;
            color: #3B82F6;
            margin: 0 30px;
        }
        .barcode {
            text-align: center;
            padding: 20px;
            background: #F8FAFC;
            border-radius: 10px;
            margin-top: 20px;
        }
        .barcode-image {
            font-family: "Libre Barcode 39", monospace;
            font-size: 48px;
            letter-spacing: 2px;
            color: #1E293B;
        }
        .footer {
            background: #F1F5F9;
            padding: 20px;
            text-align: center;
            color: #64748B;
            font-size: 12px;
        }
        .important-note {
            background: #FEF3C7;
            border-left: 4px solid #F59E0B;
            padding: 15px;
            border-radius: 8px;
            margin-top: 20px;
        }
        .important-note strong {
            color: #D97706;
        }
    </style>
</head>
<body>
    <div class="boarding-pass">
        <div class="header">
            <h1>✈️ 电子登机牌</h1>
            <div class="airline">ELECTRONIC BOARDING PASS</div>
        </div>
        
        <div class="content">
            <!-- 航线信息 -->
            <div class="flight-route">
                <div class="city">
                    <div class="city-code">)" + flight.departure.left(3) + R"(</div>
                    <div class="city-name">)" + flight.departure + R"(</div>
                    <div class="info-label" style="margin-top:10px">)" + 
                        flight.departTime.toString("yyyy-MM-dd HH:mm") + R"(</div>
                </div>
                <div class="plane-icon">✈️</div>
                <div class="city">
                    <div class="city-code">)" + flight.destination.left(3) + R"(</div>
                    <div class="city-name">)" + flight.destination + R"(</div>
                    <div class="info-label" style="margin-top:10px">)" + 
                        flight.arriveTime.toString("yyyy-MM-dd HH:mm") + R"(</div>
                </div>
            </div>

            <!-- 航班信息 -->
            <div class="section">
                <div class="section-title">航班信息 / Flight Information</div>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">航班号 Flight No.</div>
                        <div class="info-value">)" + flight.flightNumber + R"(</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">座位号 Seat</div>
                        <div class="info-value">)" + passenger.seatNumber + R"(</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">登机口 Gate</div>
                        <div class="info-value">)" + flight.gate + R"(</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">航站楼 Terminal</div>
                        <div class="info-value">)" + flight.terminal + R"(</div>
                    </div>
                </div>
            </div>

            <!-- 乘客信息 -->
            <div class="section">
                <div class="section-title">乘客信息 / Passenger Information</div>
                <div class="info-grid">
                    <div class="info-item">
                        <div class="info-label">姓名 Name</div>
                        <div class="info-value">)" + passenger.name + R"(</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">证件号码 ID Number</div>
                        <div class="info-value">)" + passenger.idCard + R"(</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">票号 Ticket No.</div>
                        <div class="info-value">)" + passenger.ticketNumber + R"(</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">预订码 Booking Code</div>
                        <div class="info-value">)" + passenger.bookingCode + R"(</div>
                    </div>
                </div>
            </div>

            <!-- 条形码 -->
            <div class="barcode">
                <div class="barcode-image">*)" + passenger.ticketNumber + R"(*</div>
            </div>

            <!-- 重要提示 -->
            <div class="important-note">
                <strong>⚠️ 重要提示：</strong><br>
                • 请提前90分钟到达机场办理值机手续<br>
                • 登机时间为起飞前30分钟，逾时不候<br>
                • 请携带有效身份证件原件登机
            </div>
        </div>

        <div class="footer">
            FlyOver Airlines | 感谢您的选择 | Have a nice flight! 🛫
        </div>
    </div>
</body>
</html>
    )";
    
    return html;
}

bool BoardingPass::exportToPDF(const QString& html, const QString& outputPath)
{
    QString filePath = outputPath;
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(nullptr, 
            "保存登机牌", 
            QDir::homePath() + "/boarding_pass.pdf",
            "PDF文件 (*.pdf)");
        
        if (filePath.isEmpty()) {
            return false;
        }
    }

    QPdfWriter pdfWriter(filePath);
    pdfWriter.setPageSize(QPageSize::A4);
    pdfWriter.setPageMargins(QMarginsF(10, 10, 10, 10));
    pdfWriter.setResolution(300);

    QPainter painter(&pdfWriter);
    if (!painter.isActive()) {
        return false;
    }

    QTextDocument document;
    document.setHtml(html);
    document.setPageSize(QSizeF(pdfWriter.width(), pdfWriter.height()));
    document.drawContents(&painter);
    painter.end();

    return true;
}

bool BoardingPass::printBoardingPass(const PassengerInfo& passenger, 
                                      const FlightInfo& flight,
                                      const QString& outputPath)
{
    QString html = generateBoardingPassHTML(passenger, flight);
    
    if (exportToPDF(html, outputPath)) {
        QMessageBox::information(nullptr, "成功", 
            "登机牌已生成！\n保存路径：" + outputPath);
        return true;
    }
    
    return false;
}
