// styleutils.h
#ifndef STYLEUTILS_H
#define STYLEUTILS_H

#include <QString>
#include <QLabel>
#include <QPushButton>

class StyleUtils
{
public:
    // 卡片样式 - 支持主题自适应
    static QString getCardStyle()
    {
        return R"(
            background-color: rgba(255, 255, 255, 200);
            border-radius: 8px;
            padding: 16px;
            border-left: 5px solid #ff9500;
        )";
    }

    // 获取按钮样式
    static QString getButtonStyle()
    {
        return R"(
            background-color: #ff9500;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: bold;
            font-size: 13px;
            min-height: 40px;
        )";
    }

    // 获取输入框样式
    static QString getInputStyle()
    {
        return R"(
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 8px;
            background-color: white;
            font-size: 12px;
            min-height: 32px;
        )";
    }

    // 应用卡片样式到标签
    static void applyCardStyle(QLabel *label, const QString& borderColor = "#ff9500")
    {
        QString style = QString(R"(
            background-color: white;
            border-radius: 8px;
            padding: 16px;
            border-left: 5px solid %1;
            color: #333;
        )").arg(borderColor);
        label->setStyleSheet(style);
    }

    // 应用按钮样式
    static void applyButtonStyle(QPushButton *button, const QString& bgColor = "#ff9500")
    {
        QString style = QString(R"(
            background-color: %1;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-weight: bold;
            min-height: 40px;
        )").arg(bgColor);
        button->setStyleSheet(style);
    }
};

#endif // STYLEUTILS_H
