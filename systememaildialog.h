#ifndef SYSTEMEMAILDIALOG_H
#define SYSTEMEMAILDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QSplitter>
#include <QComboBox>
#include "dbmanager.h"

class SystemEmailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemEmailDialog(const QString& account, QWidget *parent = nullptr);
    ~SystemEmailDialog();

private slots:
    void loadEmails();
    void onEmailSelected(QListWidgetItem *item);
    void onDeleteEmail();
    void onFilterChanged(int index); // 新增：筛选改变槽函数

private:
    QString m_account;
    QListWidget *m_emailList;
    QTextBrowser *m_contentViewer;
    QLabel *m_subjectLabel;
    QLabel *m_timeLabel;
    QLabel *m_senderLabel;
    QPushButton *m_refreshButton;
    QPushButton *m_deleteButton;
    QComboBox *m_filterCombo; // 新增：筛选下拉框
    QList<SystemEmail> m_emails; // 缓存邮件列表
    
    void setupUI();
    void updateDetailView(const SystemEmail& email);
};

#endif // SYSTEMEMAILDIALOG_H
