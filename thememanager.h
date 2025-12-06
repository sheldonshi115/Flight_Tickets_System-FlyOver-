// thememanager.h
#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QApplication>

class ThemeManager
{
public:
    enum Theme {
        Light,
        Dark
    };

    static ThemeManager& instance()
    {
        static ThemeManager s_instance;
        return s_instance;
    }

    static void setLightTheme()
    {
        instance().m_currentTheme = Light;
        qApp->setStyle("Fusion");
        qApp->setStyleSheet(getLightStyleSheet());
    }

    static void setDarkTheme()
    {
        instance().m_currentTheme = Dark;
        qApp->setStyle("Fusion");
        qApp->setStyleSheet(getDarkStyleSheet());
    }

    static bool isDarkMode()
    {
        return instance().m_currentTheme == Dark;
    }

    static QString getLightStyleSheet()
    {
        return R"(
            QMainWindow, QWidget {
                background-color: #f5f5f5;
                color: #333333;
            }
            QPushButton {
                background-color: #ff9500;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 10px 20px;
                font-weight: bold;
                font-size: 13px;
                min-height: 40px;
            }
            QPushButton:hover {
                background-color: #ff8000;
            }
            QPushButton:pressed {
                background-color: #e68a00;
            }
            QPushButton:disabled {
                background-color: #ccc;
            }
            QComboBox, QDateEdit, QLineEdit {
                border: 1px solid #ddd;
                border-radius: 6px;
                padding: 8px;
                background-color: white;
                color: #333333;
                font-size: 12px;
                min-height: 32px;
            }
            QComboBox:hover, QDateEdit:hover, QLineEdit:hover {
                border: 1px solid #0078d4;
            }
            QListWidget {
                border: 1px solid #ddd;
                border-radius: 6px;
                background-color: white;
                color: #333333;
                outline: none;
            }
            QListWidget::item:selected {
                background-color: #ff9500;
                color: white;
            }
            QLabel {
                color: #333333;
            }
            QGroupBox {
                border: 1px solid #e0e0e0;
                border-radius: 6px;
                margin-top: 12px;
                padding-top: 12px;
                color: #333333;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 3px 0 3px;
            }
            QTabWidget::pane {
                border: 1px solid #ddd;
            }
            QTabBar::tab {
                background-color: #e0e0e0;
                color: #333333;
                padding: 8px 20px;
                border: 1px solid #ddd;
                border-radius: 6px 6px 0 0;
            }
            QTabBar::tab:selected {
                background-color: #ffffff;
                color: #333333;
                border: 1px solid #ddd;
                border-bottom: 3px solid #ff9500;
            }
            QTabBar::tab:hover {
                background-color: #f0f0f0;
            }
            QMenuBar {
                background-color: #ffffff;
                color: #333333;
                border-bottom: 1px solid #ddd;
            }
            QMenuBar::item:selected {
                background-color: #ff9500;
                color: white;
            }
            QMenu {
                background-color: #ffffff;
                color: #333333;
                border: 1px solid #ddd;
            }
            QMenu::item:selected {
                background-color: #ff9500;
                color: white;
            }
        )";
    }

    static QString getDarkStyleSheet()
    {
        return R"(
            QMainWindow, QWidget {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            QPushButton {
                background-color: #0078d4;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 10px 20px;
                font-weight: bold;
                font-size: 13px;
                min-height: 40px;
            }
            QPushButton:hover {
                background-color: #106ebe;
            }
            QPushButton:pressed {
                background-color: #005a9e;
            }
            QPushButton:disabled {
                background-color: #555;
            }
            QComboBox, QDateEdit, QLineEdit {
                border: 1px solid #444;
                border-radius: 6px;
                padding: 8px;
                background-color: #2d2d2d;
                color: #e0e0e0;
                font-size: 12px;
                min-height: 32px;
            }
            QComboBox:hover, QDateEdit:hover, QLineEdit:hover {
                border: 1px solid #0078d4;
            }
            QListWidget {
                border: 1px solid #444;
                border-radius: 6px;
                background-color: #2d2d2d;
                color: #e0e0e0;
                outline: none;
            }
            QListWidget::item:selected {
                background-color: #0078d4;
                color: white;
            }
            QLabel {
                color: #e0e0e0;
            }
            QGroupBox {
                border: 1px solid #444;
                border-radius: 6px;
                margin-top: 12px;
                padding-top: 12px;
                color: #e0e0e0;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 3px 0 3px;
            }
            QTabWidget::pane {
                border: 1px solid #444;
            }
            QTabBar::tab {
                background-color: #2d2d2d;
                color: #e0e0e0;
                padding: 8px 20px;
                border: 1px solid #444;
                border-radius: 6px 6px 0 0;
            }
            QTabBar::tab:selected {
                background-color: #1e1e1e;
                color: #e0e0e0;
                border: 1px solid #444;
                border-bottom: 3px solid #0078d4;
            }
            QTabBar::tab:hover {
                background-color: #3d3d3d;
            }
            QMenuBar {
                background-color: #2d2d2d;
                color: #e0e0e0;
                border-bottom: 1px solid #444;
            }
            QMenuBar::item:selected {
                background-color: #0078d4;
                color: white;
            }
            QMenu {
                background-color: #2d2d2d;
                color: #e0e0e0;
                border: 1px solid #444;
            }
            QMenu::item:selected {
                background-color: #0078d4;
                color: white;
            }
        )";
    }

private:
    ThemeManager() : m_currentTheme(Light) {}
    Theme m_currentTheme;
};

#endif // THEMEMANAGER_H
