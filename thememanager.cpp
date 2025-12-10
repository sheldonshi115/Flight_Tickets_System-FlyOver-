// thememanager.cpp
#include "thememanager.h"
#include <QApplication>
#include <QWidget>
#include <QStyle>
#include <QPalette>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_isDarkMode(false), m_stylesInitialized(false)
{
    // 延迟初始化样式，仅在第一次使用时缓存
    // 默认浅色主题 (m_isDarkMode = false)
}

bool ThemeManager::isDarkMode() const
{
    return m_isDarkMode;
}

void ThemeManager::setDarkMode(bool dark)
{
    if (m_isDarkMode != dark) {
        m_isDarkMode = dark;
        applyTheme();
        emit themeChanged(m_isDarkMode);
    }
}

void ThemeManager::toggleTheme()
{
    setDarkMode(!m_isDarkMode);
}

void ThemeManager::forceLightTheme()
{
    // 强制设置为浅色主题
    m_isDarkMode = false;
    
    // 如果样式未初始化，先初始化缓存
    if (!m_stylesInitialized) {
        initializeStyles();
    }
    
    // 强制应用浅色主题样式 - 优雅柔和配色
    if (qApp) {
        qApp->setStyle("Fusion");
        qApp->setStyleSheet(m_cachedLightStyle);
        
        // 设置QPalette确保覆盖系统主题 - 去除黑色使用柔和灰色
        QPalette lightPalette;
        lightPalette.setColor(QPalette::Window, QColor("#F8FAFC"));
        lightPalette.setColor(QPalette::WindowText, QColor("#475569"));  // 柔和深灰色替代黑色
        lightPalette.setColor(QPalette::Base, QColor("#FFFFFF"));
        lightPalette.setColor(QPalette::AlternateBase, QColor("#F1F5F9"));
        lightPalette.setColor(QPalette::Text, QColor("#475569"));  // 柔和深灰色替代黑色
        lightPalette.setColor(QPalette::Button, QColor("#FFFFFF"));
        lightPalette.setColor(QPalette::ButtonText, QColor("#475569"));  // 柔和深灰色替代黑色
        lightPalette.setColor(QPalette::BrightText, Qt::white);
        lightPalette.setColor(QPalette::Link, QColor("#3B82F6"));  // 现代蓝色
        lightPalette.setColor(QPalette::Highlight, QColor("#60A5FA"));  // 柔和蓝色高亮
        lightPalette.setColor(QPalette::HighlightedText, QColor("#1E40AF"));  // 深蓝色文字
        
        qApp->setPalette(lightPalette);
        qApp->processEvents();
    }
    
    emit themeChanged(false);
}

QString ThemeManager::primaryColor() const
{
    return m_isDarkMode ? "#3498DB" : "#0A3C5F";  // 深海蓝 → 天空青
}

QString ThemeManager::accentColor() const
{
    return m_isDarkMode ? "#5DADE2" : "#2980B9";  // 蓝色强调
}

QString ThemeManager::backgroundColor() const
{
    return m_isDarkMode ? "#121212" : "#F8FAFC";  // 柔和浅灰色背景
}

QString ThemeManager::surfaceColor() const
{
    return m_isDarkMode ? "#1E1E1E" : "#FFFFFF";
}

QString ThemeManager::textColor() const
{
    return m_isDarkMode ? "#E0E0E0" : "#475569";  // 柔和深灰色文字（去除黑色）
}

QString ThemeManager::secondaryTextColor() const
{
    return m_isDarkMode ? "#9E9E9E" : "#64748B";  // 柔和次要文字色
}

QString ThemeManager::borderColor() const
{
    return m_isDarkMode ? "#2D2D2D" : "#E2E8F0";  // 柔和边框色
}

QString ThemeManager::successColor() const
{
    return "#4CAF50";  // 绿色 - 可用座位
}

QString ThemeManager::errorColor() const
{
    return "#F44336";  // 红色
}

QString ThemeManager::warningColor() const
{
    return "#FF9800";  // 橙色
}

void ThemeManager::applyTheme()
{
    // 如果样式未初始化，先初始化缓存
    if (!m_stylesInitialized) {
        initializeStyles();
    }
    
    // 直接使用缓存的样式
    QString styleToApply = m_isDarkMode ? m_cachedDarkStyle : m_cachedLightStyle;
    
    // 应用样式到全局应用程序
    if (qApp) {
        qApp->setStyle("Fusion");
        qApp->setStyleSheet(styleToApply);
        
        // 处理事件以保持响应性
        qApp->processEvents();
    }
}

void ThemeManager::initializeStyles()
{
    // ========== 深色主题样式缓存 ==========
    m_cachedDarkStyle = R"(
            /* 全局基础样式 - 深色主题 */
            * {
                font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            }
            
            QMainWindow, QDialog, QWidget {
                background-color: #1E1E1E;
                color: #FFFFFF;
            }
            
            /* 中央内容区域（关键修复） */
            QStackedWidget, QStackedWidget > QWidget {
                background-color: #2C2C2C;
            }
            
            /* 页面容器 */
            QWidget#pageHome, QWidget#pageFlightQuery {
                background-color: #2C2C2C;
            }
            
            /* 对话框优化 */
            QDialog {
                background-color: #1E293B;
                border: 1px solid #334155;
            }
            
            /* 分组框（关键修复） */
            QGroupBox {
                background-color: #1E293B;
                border: 1px solid #334155;
                border-radius: 16px;
                margin-top: 16px;
                padding: 20px;
                color: #E2E8F0;
                font-size: 15px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 12px;
                color: #60A5FA;
                font-weight: 600;
            }
            
            /* 消息框按钮优化 */
            QMessageBox QPushButton {
                background-color: #3B82F6;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 20px;
                min-width: 80px;
            }
            QMessageBox QPushButton:hover {
                background-color: #60A5FA;
            }
            QMessageBox QPushButton:pressed {
                background-color: #2563EB;
            }
            
            /* 侧边栏 */
            QFrame#sideBar {
                background-color: #1E1E1E;
                border-right: 1px solid #2D2D2D;
            }
            QFrame#sideBar QPushButton {
                background-color: transparent;
                color: #E0E0E0;
                border: none;
                border-radius: 8px;
                text-align: left;
                padding: 12px 20px;
                font-size: 14px;
            }
            QFrame#sideBar QPushButton:hover {
                background-color: rgba(66, 165, 245, 0.15);
            }
            QFrame#sideBar QPushButton:pressed {
                background-color: rgba(66, 165, 245, 0.25);
            }
            
            /* Logo */
            QLabel#logoLabel {
                color: #64B5F6;
                font-size: 22px;
                font-weight: bold;
                background: transparent;
            }
            
            /* 按钮 */
            QPushButton {
                background-color: #3B82F6;
                color: white;
                border: none;
                border-radius: 12px;
                padding: 12px 24px;
                font-size: 14px;
                font-weight: 500;
            }
            QPushButton:hover {
                background-color: #60A5FA;
            }
            QPushButton:pressed {
                background-color: #2563EB;
            }
            QPushButton:disabled {
                background-color: #334155;
                color: #64748B;
            }
            
            /* 输入框 */
            QLineEdit, QTextEdit, QDateEdit, QComboBox, QSpinBox {
                background-color: #1E293B;
                color: #E2E8F0;
                border: 2px solid #334155;
                border-radius: 10px;
                padding: 10px 14px;
                font-size: 13px;
                selection-background-color: #3B82F6;
                selection-color: white;
            }
            QLineEdit:focus, QTextEdit:focus, QDateEdit:focus, QComboBox:focus, QSpinBox:focus {
                border-color: #60A5FA;
                background-color: #0F172A;
            }
            QLineEdit:disabled, QTextEdit:disabled, QDateEdit:disabled, QComboBox:disabled, QSpinBox:disabled {
                background-color: #0F172A;
                color: #475569;
                border-color: #1E293B;
            }
            
            /* 下拉框箭头 */
            QComboBox::drop-down {
                border: none;
                width: 30px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 6px solid #64748B;
                margin-right: 8px;
            }
            QComboBox QAbstractItemView {
                background-color: #FFFFFF;
                color: #475569;
                border: 1px solid #E2E8F0;
                selection-background-color: #DBEAFE;
                selection-color: #1E40AF;
                border-radius: 8px;
                padding: 4px;
            }
            
            /* 表格 */
            QTableWidget, QTableView {
                background-color: #1E293B;
                alternate-background-color: #0F172A;
                gridline-color: #334155;
                color: #E2E8F0;
                border: 1px solid #334155;
                border-radius: 12px;
            }
            QHeaderView::section {
                background-color: #0F172A;
                color: #E2E8F0;
                border: none;
                border-bottom: 2px solid #3B82F6;
                border-radius: 0px;
                padding: 12px;
                font-weight: 600;
            }
            QTableWidget::item:selected, QTableView::item:selected {
                background-color: #3B82F6;
                color: white;
            }
            QTableWidget::item:hover, QTableView::item:hover {
                background-color: #1E293B;
            }
                font-weight: bold;
                padding: 12px 8px;
                border: none;
                border-bottom: 2px solid #42A5F5;
            }
            
            /* 菜单栏 */
            QMenuBar {
                background-color: #1E1E1E;
                color: #E0E0E0;
                border-bottom: 1px solid #2D2D2D;
            }
            QMenuBar::item:selected {
                background-color: rgba(66, 165, 245, 0.3);
            }
            QMenu {
                background-color: #252525;
                color: #E0E0E0;
                border: 1px solid #2D2D2D;
            }
            QMenu::item:selected {
                background-color: rgba(66, 165, 245, 0.3);
            }
            
            /* 列表 */
            QListWidget {
                background-color: #1E1E1E;
                color: #E0E0E0;
                border: 1px solid #2D2D2D;
                border-radius: 8px;
            }
            QListWidget::item {
                padding: 12px 16px;
                border-bottom: 1px solid #2D2D2D;
            }
            QListWidget::item:hover {
                background-color: rgba(66, 165, 245, 0.1);
            }
            QListWidget::item:selected {
                background-color: rgba(66, 165, 245, 0.3);
            }
            
            /* 标签页 */
            QTabWidget::pane {
                border: 1px solid #2D2D2D;
                border-radius: 8px;
                background-color: #1E1E1E;
            }
            QTabBar::tab {
                background-color: #252525;
                color: #9E9E9E;
                padding: 10px 20px;
                margin-right: 4px;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
            }
            QTabBar::tab:selected {
                background-color: #1E1E1E;
                color: #64B5F6;
                font-weight: bold;
            }
            
            /* 分组框 */
            QGroupBox {
                background-color: #1E1E1E;
                border: 1px solid #2D2D2D;
                border-radius: 8px;
                margin-top: 16px;
                padding: 16px;
                color: #E0E0E0;
            }
            QGroupBox::title {
                color: #64B5F6;
            }
            
            /* 滚动条 - 垂直 */
            QScrollBar:vertical {
                background-color: #1E293B;
                width: 12px;
                border-radius: 6px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background-color: #334155;
                border-radius: 6px;
                min-height: 30px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #60A5FA;
            }
            QScrollBar::handle:vertical:pressed {
                background-color: #3B82F6;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                background: none;
            }
            
            /* 滚动条 - 水平 */
            QScrollBar:horizontal {
                background-color: #1E1E1E;
                height: 12px;
                border-radius: 6px;
                margin: 0;
            }
            QScrollBar::handle:horizontal {
                background-color: #424242;
                border-radius: 6px;
                min-width: 30px;
            }
            QScrollBar::handle:horizontal:hover {
                background-color: #5DADE2;
            }
            QScrollBar::handle:horizontal:pressed {
                background-color: #3498DB;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                width: 0px;
            }
            QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
                background: none;
            }
            
            /* QScrollArea - 确保不显示白色背景 */
            QScrollArea {
                background-color: #0F172A;
                border: none;
            }
            QScrollArea > QWidget > QWidget {
                background-color: #0F172A;
            }
            
            /* 状态栏 */
            QStatusBar {
                background-color: #1E1E1E;
                color: #757575;
            }
            
            /* 工具提示 */
            QToolTip {
                background-color: #1E293B;
                color: #E2E8F0;
                border: 1px solid #3B82F6;
                border-radius: 4px;
                padding: 6px 10px;
                font-size: 12px;
            }
            
            /* 复选框和单选框 */
            QCheckBox, QRadioButton {
                color: #E2E8F0;
                spacing: 8px;
            }
            QCheckBox::indicator, QRadioButton::indicator {
                width: 18px;
                height: 18px;
                border: 2px solid #334155;
                border-radius: 3px;
                background-color: #1E293B;
            }
            QCheckBox::indicator:checked, QRadioButton::indicator:checked {
                background-color: #3B82F6;
                border-color: #3B82F6;
            }
            QCheckBox::indicator:hover, QRadioButton::indicator:hover {
                border-color: #60A5FA;
            }
            QCheckBox:disabled, QRadioButton:disabled {
                color: #666666;
            }
        )";
    
    // ========== 浅色主题样式缓存 ==========
    m_cachedLightStyle = R"(
            /* 全局基础样式 - 高级温柔浅色主题 */
            * {
                font-family: "Microsoft YaHei UI", "Segoe UI", "SF Pro Display", sans-serif;
            }
            
            QMainWindow, QDialog, QWidget {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #F8FAFC, stop:1 #E8EAF6);
                color: #1E293B;
            }
            
            /* 中央内容区域 */
            QStackedWidget, QStackedWidget > QWidget {
                background-color: #FAFAFA;
            }
            
            /* 页面容器 */
            QWidget#pageHome, QWidget#pageFlightQuery {
                background-color: #FAFAFA;
            }
            
            /* 对话框优化 */
            QDialog {
                background-color: #FFFFFF;
                border: 1px solid #E0E0E0;
            }
            
            /* 分组框 */
            QGroupBox {
                background-color: #FFFFFF;
                border: 2px solid #E0E0E0;
                border-radius: 12px;
                margin-top: 16px;
                padding: 20px;
                color: #212121;
                font-size: 15px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 12px;
                color: #1976D2;
                font-weight: 600;
            }
            
            /* 中央内容区域（关键修复） */
            QStackedWidget, QStackedWidget > QWidget {
                background-color: #F9FAFB;
            }
            
            /* 页面容器 */
            QWidget#pageHome, QWidget#pageFlightQuery {
                background-color: #F9FAFB;
            }
            
            /* 对话框优化 */
            QDialog {
                background-color: #FFFFFF;
                border: 1px solid #E5E7EB;
            }
            
            /* 分组框（关键修复） */
            QGroupBox {
                background-color: #FFFFFF;
                border: 1px solid #E5E7EB;
                border-radius: 16px;
                margin-top: 16px;
                padding: 20px;
                color: #1F2937;
                font-size: 15px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 12px;
                color: #3B82F6;
                font-weight: 600;
            }
            
            /* 对话框优化 */
            QDialog {
                background-color: #FFFFFF;
                border: 1px solid #E0E0E0;
            }
            
            /* 消息框按钮优化 */
            QMessageBox QPushButton {
                background-color: #3B82F6;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 8px 20px;
                min-width: 80px;
            }
            QMessageBox QPushButton:hover {
                background-color: #60A5FA;
            }
            QMessageBox QPushButton:pressed {
                background-color: #2563EB;
            }
            
            /* 侧边栏 - 带文字模式 */
            QFrame#sideBar {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #F8FAFC, stop:1 #EFF6FF);
                border-right: 1px solid rgba(0, 0, 0, 0.06);
            }
            QFrame#sideBar QPushButton {
                background-color: transparent;
                color: #64748B;
                border: 2px solid transparent;
                border-radius: 8px;
                font-size: 15px;
                padding: 12px 20px;
                margin: 2px;
                text-align: left;
            }
            QFrame#sideBar QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #DBEAFE, stop:1 #BFDBFE);
                border-color: #BFDBFE;
                color: #1E40AF;
            }
            QFrame#sideBar QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #BFDBFE, stop:1 #93C5FD);
                border-color: #60A5FA;
                color: #1E40AF;
            }
            
            /* Logo - 鲜明蓝色渐变 */
            QLabel#logoLabel {
                color: #2563EB;
                font-size: 26px;
                font-weight: bold;
                background: transparent;
                letter-spacing: 1px;
            }
            
            /* 按钮样式 - 高级渐变 */
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #42A5F5, stop:1 #1E88E5);
                color: white;
                border: none;
                border-radius: 12px;
                padding: 12px 28px;
                font-size: 14px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #64B5F6, stop:1 #2196F3);
            }
            QPushButton:pressed {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #1E88E5, stop:1 #1565C0);
            }
            QPushButton:disabled {
                background: #E0E0E0;
                color: #9E9E9E;
            }
            
            /* 输入框和日期选择器 - 柔和风格 */
            QLineEdit, QTextEdit, QDateEdit, QComboBox, QSpinBox {
                background-color: #FFFFFF;
                color: #475569;
                border: 2px solid #E2E8F0;
                border-radius: 10px;
                padding: 12px 16px;
                font-size: 15px;
                font-family: 'Microsoft YaHei UI', 'SimHei';
                selection-background-color: #BFDBFE;
                selection-color: #1E40AF;
            }
            QLineEdit:focus, QTextEdit:focus, QDateEdit:focus, QComboBox:focus, QSpinBox:focus {
                border-color: #60A5FA;
                background-color: #F0F9FF;
                /* Qt不支持box-shadow，使用border代替光晕效果 */
                border: 3px solid #BFDBFE;
            }
            QDateEdit::drop-down {
                border: none;
                width: 30px;
                subcontrol-origin: padding;
                subcontrol-position: center right;
            }
            QDateEdit::down-arrow {
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 6px solid #3B82F6;
                margin-right: 8px;
            }
            
            /* 日历弹窗 - 优雅现代风格 */
            QCalendarWidget {
                background-color: #FFFFFF;
                border: 2px solid #E2E8F0;
                border-radius: 12px;
            }
            
            /* 日历导航栏 */
            QCalendarWidget QWidget#qt_calendar_navigationbar {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #60A5FA, stop:1 #3B82F6);
                border-top-left-radius: 10px;
                border-top-right-radius: 10px;
            }
            
            /* 月份/年份按钮 */
            QCalendarWidget QToolButton {
                color: #FFFFFF;
                background-color: transparent;
                border: none;
                border-radius: 6px;
                padding: 8px 12px;
                margin: 4px;
                font-size: 14px;
                font-weight: 600;
            }
            QCalendarWidget QToolButton:hover {
                background-color: rgba(255, 255, 255, 0.2);
            }
            QCalendarWidget QToolButton:pressed {
                background-color: rgba(255, 255, 255, 0.3);
            }
            QCalendarWidget QToolButton::menu-indicator {
                image: none;
            }
            
            /* 上一个/下一个月按钮 */
            QCalendarWidget QToolButton#qt_calendar_prevmonth,
            QCalendarWidget QToolButton#qt_calendar_nextmonth {
                qproperty-icon: none;
                min-width: 36px;
                max-width: 36px;
                min-height: 36px;
                max-height: 36px;
                border-radius: 18px;
            }
            QCalendarWidget QToolButton#qt_calendar_prevmonth:hover,
            QCalendarWidget QToolButton#qt_calendar_nextmonth:hover {
                background-color: rgba(255, 255, 255, 0.25);
            }
            
            /* 月份选择按钮 */
            QCalendarWidget QToolButton#qt_calendar_monthbutton,
            QCalendarWidget QToolButton#qt_calendar_yearbutton {
                font-size: 15px;
                font-weight: bold;
                min-width: 80px;
            }
            
            /* 星期标题栏 */
            QCalendarWidget QWidget {
                alternate-background-color: #F8FAFC;
            }
            
            QCalendarWidget QAbstractItemView:enabled {
                background-color: #FFFFFF;
                color: #475569;
                selection-background-color: #3B82F6;
                selection-color: #FFFFFF;
                font-size: 13px;
            }
            
            /* 日期单元格 */
            QCalendarWidget QAbstractItemView {
                gridline-color: rgba(226, 232, 240, 0.3);
                outline: none;
            }
            
            /* 今天日期 */
            QCalendarWidget QAbstractItemView:enabled {
                color: #475569;
            }
            
            /* 其他月份日期 */
            QCalendarWidget QAbstractItemView:disabled {
                color: #CBD5E1;
            }
            
            /* 周末日期 */
            QCalendarWidget QAbstractItemView {
                color: #475569;
            }
            
            QLineEdit:disabled, QTextEdit:disabled, QDateEdit:disabled, QComboBox:disabled, QSpinBox:disabled {
                background-color: #F1F5F9;
                color: #94A3B8;
                border-color: #E2E8F0;
            }
            QLineEdit:disabled, QTextEdit:disabled, QDateEdit:disabled, QComboBox:disabled, QSpinBox:disabled {
                background-color: #F9FAFB;
                color: #6B7280;
                border-color: #E5E7EB;
            }
            
            /* 下拉框箭头 */
            QComboBox::drop-down {
                border: none;
                width: 30px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 5px solid transparent;
                border-right: 5px solid transparent;
                border-top: 6px solid #64748B;
                margin-right: 8px;
            }
            QComboBox QAbstractItemView {
                background-color: #FFFFFF;
                color: #475569;
                border: 1px solid #E2E8F0;
                selection-background-color: #DBEAFE;
                selection-color: #1E40AF;
                border-radius: 8px;
                padding: 4px;
            }
            
            /* 表格 - 炫酷科技风格 */
            QTableWidget, QTableView {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #FFFFFF, stop:1 #F8FAFC);
                alternate-background-color: #F1F5F9;
                gridline-color: transparent;
                color: #475569;
                border: 1px solid #E2E8F0;
                border-radius: 12px;
                font-size: 13px;
            }
            QHeaderView::section {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #60A5FA, stop:1 #3B82F6);
                color: #FFFFFF;
                border: none;
                border-right: 1px solid rgba(255, 255, 255, 0.1);
                border-radius: 0px;
                padding: 14px 12px;
                font-weight: 700;
                font-size: 13px;
                text-transform: uppercase;
                letter-spacing: 0.5px;
            }
            QTableWidget::item, QTableView::item {
                padding: 14px 12px;
                border-bottom: 1px solid #E2E8F0;
                color: #475569;
            }
            QTableWidget::item:selected, QTableView::item:selected {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #DBEAFE, stop:1 #BFDBFE);
                color: #1E40AF;
                font-weight: 600;
            }
            QTableWidget::item:hover, QTableView::item:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #F0F9FF, stop:1 #E0F2FE);
            }
            }
            QTableWidget::item {
                padding: 12px 8px;
                border-bottom: 1px solid #F5F5F5;
            }
            QTableWidget::item:selected {
                background-color: #D6EAF8;
                color: #0A3C5F;
            }
            QHeaderView::section {
                background-color: #F5F5F5;
                color: #616161;
                font-weight: bold;
                padding: 12px 8px;
                border: none;
                border-bottom: 2px solid #3498DB;
            }
            
            /* 菜单栏 - 柔和蓝色渐变风格 */
            QMenuBar {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #FFFFFF, stop:1 #F0F4F8);
                color: #475569;
                border-bottom: 1px solid rgba(0, 0, 0, 0.05);
                padding: 6px 0;
            }
            QMenuBar::item {
                background: transparent;
                color: #475569;
                padding: 8px 16px;
                border-radius: 6px;
                margin: 0 4px;
            }
            QMenuBar::item:selected {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #E0F2FE, stop:1 #BAE6FD);
                color: #0369A1;
            }
            QMenu {
                background-color: #FFFFFF;
                color: #475569;
                border: 1px solid rgba(0, 0, 0, 0.08);
                border-radius: 8px;
                padding: 8px;
            }
            QMenu::item {
                padding: 10px 24px;
                border-radius: 6px;
            }
            QMenu::item:selected {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #E0F2FE, stop:1 #BAE6FD);
                color: #0369A1;
            }
            
            /* 列表 */
            QListWidget {
                background-color: #FFFFFF;
                color: #475569;
                border: 1px solid #E2E8F0;
                border-radius: 12px;
            }
            QListWidget::item {
                padding: 14px 18px;
                border-bottom: 1px solid #F1F5F9;
            }
            QListWidget::item:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #F0F9FF, stop:1 #E0F2FE);
            }
            QListWidget::item:selected {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #DBEAFE, stop:1 #BFDBFE);
                color: #1E40AF;
                font-weight: 600;
            }
            
            /* 标签页 - 现代渐变风格 */
            QTabWidget::pane {
                border: 1px solid #E2E8F0;
                border-radius: 12px;
                background-color: #FFFFFF;
            }
            QTabBar::tab {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #F8FAFC, stop:1 #F1F5F9);
                color: #64748B;
                padding: 12px 24px;
                margin-right: 4px;
                border-top-left-radius: 10px;
                border-top-right-radius: 10px;
                font-weight: 500;
            }
            QTabBar::tab:selected {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #DBEAFE, stop:1 #BFDBFE);
                color: #1E40AF;
                font-weight: 700;
            }
            QTabBar::tab:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #E0F2FE, stop:1 #BAE6FD);
                color: #0369A1;
            }
            
            /* 分组框 - 柔和渐变风格 */
            QGroupBox {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #FFFFFF, stop:1 #F8FAFC);
                border: 2px solid #E2E8F0;
                border-radius: 12px;
                margin-top: 20px;
                padding: 20px 16px 16px 16px;
                color: #475569;
                font-size: 14px;
            }
            QGroupBox::title {
                color: #3B82F6;
                font-weight: 700;
                font-size: 15px;
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 4px 12px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #DBEAFE, stop:1 #BFDBFE);
                border-radius: 8px;
                left: 12px;
            }
            
            /* 滚动条 - 垂直 */
            QScrollBar:vertical {
                background-color: #F9FAFB;
                width: 12px;
                border-radius: 6px;
                margin: 0;
            }
            QScrollBar::handle:vertical {
                background-color: #D1D5DB;
                border-radius: 6px;
                min-height: 30px;
            }
            QScrollBar::handle:vertical:hover {
                background-color: #60A5FA;
            }
            QScrollBar::handle:vertical:pressed {
                background-color: #3B82F6;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                background: none;
            }
            
            /* 滚动条 - 水平 */
            QScrollBar:horizontal {
                background-color: #F5F5F5;
                height: 12px;
                border-radius: 6px;
                margin: 0;
            }
            QScrollBar::handle:horizontal {
                background-color: #BDBDBD;
                border-radius: 6px;
                min-width: 30px;
            }
            QScrollBar::handle:horizontal:hover {
                background-color: #3498DB;
            }
            QScrollBar::handle:horizontal:pressed {
                background-color: #0A3C5F;
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                width: 0px;
            }
            QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
                background: none;
            }
            
            /* QScrollArea - 确保不显示白色背景 */
            QScrollArea {
                background-color: #FFFFFF;
                border: none;
            }
            QScrollArea > QWidget > QWidget {
                background-color: #FFFFFF;
            }
            
            /* 状态栏 */
            QStatusBar {
                background-color: #FFFFFF;
                color: #757575;
            }
            
            /* 工具提示 */
            QToolTip {
                background-color: #FFFFFF;
                color: #0F172A;
                border: 1px solid #60A5FA;
                border-radius: 4px;
                padding: 6px 10px;
                font-size: 12px;
            }
            
            /* 复选框和单选框 */
            QCheckBox, QRadioButton {
                color: #0F172A;
                spacing: 8px;
            }
            QCheckBox::indicator, QRadioButton::indicator {
                width: 18px;
                height: 18px;
                border: 2px solid #D1D5DB;
                border-radius: 3px;
                background-color: #FFFFFF;
            }
            QCheckBox::indicator:checked, QRadioButton::indicator:checked {
                background-color: #3B82F6;
                border-color: #3B82F6;
            }
            QCheckBox::indicator:hover, QRadioButton::indicator:hover {
                border-color: #60A5FA;
            }
            QCheckBox:disabled, QRadioButton:disabled {
                color: #9E9E9E;
            }
        )";
    
    // 标记样式已初始化
    m_stylesInitialized = true;
}

QString ThemeManager::getCardStyle() const
{
    if (m_isDarkMode) {
        return R"(
            background-color: #1E1E1E;
            border: 1px solid #2D2D2D;
            border-radius: 12px;
            padding: 20px;
        )";
    } else {
        return R"(
            background-color: #FFFFFF;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
            padding: 20px;
        )";
    }
}

QString ThemeManager::getGlassStyle() const
{
    // 磨砂玻璃效果
    if (m_isDarkMode) {
        return R"(
            background-color: rgba(30, 30, 30, 0.85);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 16px;
        )";
    } else {
        return R"(
            background-color: rgba(255, 255, 255, 0.85);
            border: 1px solid rgba(0, 0, 0, 0.1);
            border-radius: 16px;
        )";
    }
}

QString ThemeManager::getGradientButtonStyle() const
{
    return R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #42A5F5, stop:1 #673AB7);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #64B5F6, stop:1 #7E57C2);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1E88E5, stop:1 #5E35B1);
        }
    )";
}
