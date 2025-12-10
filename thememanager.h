// thememanager.h - 深海之光全局主题管理器
#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

/**
 * @brief 全局主题管理器 - 深海之光主题系统
 * 
 * 支持浅色模式（天空蓝 #42A5F5）和深色模式（深蓝灰 #121212）
 * 使用单例模式，在整个应用程序中保持主题一致性
 */
class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString primaryColor READ primaryColor NOTIFY themeChanged)
    Q_PROPERTY(QString accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY themeChanged)
    Q_PROPERTY(bool isDarkMode READ isDarkMode NOTIFY themeChanged)
    
public:
    // 单例访问
    static ThemeManager& instance();
    
    // 禁止复制
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    
    // 主题状态
    bool isDarkMode() const;
    void setDarkMode(bool dark);
    void toggleTheme();
    void forceLightTheme(); // 强制应用浅色主题，不受系统影响
    
    // 主题颜色获取
    QString primaryColor() const;      // 主色调
    QString accentColor() const;        // 强调色
    QString backgroundColor() const;   // 背景色
    QString surfaceColor() const;      // 表面色
    QString textColor() const;         // 文字色
    QString secondaryTextColor() const;// 次级文字色
    QString borderColor() const;       // 边框色
    QString successColor() const;      // 成功色（绿色）
    QString errorColor() const;        // 错误色（红色）
    QString warningColor() const;      // 警告色（橙色）
    
    // 特殊样式获取
    QString getCardStyle() const;          // 卡片样式
    QString getGlassStyle() const;         // 磨砂玻璃效果
    QString getGradientButtonStyle() const;// 渐变按钮样式
    
    // 应用主题到全局
    void applyTheme();
    
signals:
    void themeChanged(bool isDark);
    
private:
    explicit ThemeManager(QObject *parent = nullptr);
    bool m_isDarkMode;
    QString m_cachedDarkStyle;   // 缓存深色主题样式
    QString m_cachedLightStyle;  // 缓存浅色主题样式
    bool m_stylesInitialized;    // 样式是否已初始化
    
    void initializeStyles();     // 初始化并缓存样式
};

#endif // THEMEMANAGER_H
