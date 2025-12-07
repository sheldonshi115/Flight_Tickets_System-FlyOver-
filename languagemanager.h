#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QMap>
#include <QApplication>

// 语言类型枚举
enum class Language {
    Chinese,    // 中文
    English     // 英文
};

// 语言管理器类（单例模式）
class LanguageManager : public QObject
{
    Q_OBJECT
    
public:
    static LanguageManager& instance();
    
    // 切换语言
    void switchLanguage(Language lang);
    
    // 获取当前语言
    Language currentLanguage() const { return m_currentLanguage; }
    
    // 获取翻译文本
    QString tr(const QString& key) const;
    
signals:
    void languageChanged(Language lang);

private:
    explicit LanguageManager(QObject* parent = nullptr);
    ~LanguageManager();
    
    void initTranslations();
    
    Language m_currentLanguage;
    QTranslator* m_translator;
    QMap<QString, QMap<Language, QString>> m_translations;
};

#endif // LANGUAGEMANAGER_H
