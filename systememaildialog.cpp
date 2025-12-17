#include "systememaildialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QDateTime>
#include <QFrame>
#include <QIcon>
#include <QDebug>

SystemEmailDialog::SystemEmailDialog(const QString& account, QWidget *parent)
    : QDialog(parent), m_account(account)
{
    setWindowTitle("系统邮箱");
    resize(900, 600);
    setupUI();
    loadEmails();
}

SystemEmailDialog::~SystemEmailDialog()
{
}

void SystemEmailDialog::setupUI()
{
    // 设置整体样式
    this->setStyleSheet(R"(
        QDialog {
            background-color: #F3F4F6;
        }
        QListWidget {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 8px;
            outline: none;
        }
        QListWidget::item {
            height: 70px;
            border-bottom: 1px solid #F3F4F6;
            padding: 5px;
            color: #374151;
        }
        QListWidget::item:selected {
            background-color: #EFF6FF;
            color: #1E40AF;
            border-left: 4px solid #3B82F6;
        }
        QListWidget::item:hover {
            background-color: #F9FAFB;
        }
        QTextBrowser {
            background-color: #FFFFFF;
            border: 1px solid #E5E7EB;
            border-radius: 8px;
            padding: 15px;
            font-size: 14px;
            font-family: "Microsoft YaHei", "SimHei", sans-serif;
            color: #374151;
        }
        QLabel#SubjectLabel {
            font-size: 18px;
            font-weight: bold;
            color: #111827;
        }
        QLabel#MetaLabel {
            color: #6B7280;
            font-size: 13px;
        }
        QPushButton {
            background-color: #FFFFFF;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 6px 12px;
            color: #374151;
        }
        QPushButton:hover {
            background-color: #F3F4F6;
            border-color: #9CA3AF;
        }
    )");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // 左侧列表区域
    QVBoxLayout *leftLayout = new QVBoxLayout();
    
    // 顶部工具栏（左侧）
    QHBoxLayout *leftHeader = new QHBoxLayout();
    QLabel *listTitle = new QLabel("收件箱", this);
    listTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #374151;");
    
    // 筛选下拉框
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItems({"全部", "未读", "已读"});
    m_filterCombo->setFixedWidth(120); // 增加宽度以显示完整文字
    // 设置下拉框样式，确保文字居中且不被遮挡
    m_filterCombo->setStyleSheet("QComboBox { padding: 5px; } QComboBox::drop-down { width: 20px; }");
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SystemEmailDialog::onFilterChanged);

    m_refreshButton = new QPushButton("刷新", this);
    connect(m_refreshButton, &QPushButton::clicked, this, &SystemEmailDialog::loadEmails);
    
    leftHeader->addWidget(listTitle);
    leftHeader->addWidget(m_filterCombo);
    leftHeader->addStretch();
    leftHeader->addWidget(m_refreshButton);
    leftLayout->addLayout(leftHeader);

    // 邮件列表
    m_emailList = new QListWidget(this);
    m_emailList->setFixedWidth(300);
    m_emailList->setSelectionMode(QAbstractItemView::SingleSelection);
    // 使用 itemClicked 信号
    connect(m_emailList, &QListWidget::itemClicked, this, &SystemEmailDialog::onEmailSelected);
    leftLayout->addWidget(m_emailList);

    // 右侧详情区域
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    // 详情头部
    QFrame *detailHeader = new QFrame(this);
    detailHeader->setStyleSheet("background-color: #FFFFFF; border-radius: 8px; border: 1px solid #E5E7EB;");
    QVBoxLayout *headerLayout = new QVBoxLayout(detailHeader);
    
    m_subjectLabel = new QLabel("请选择一封邮件", detailHeader);
    m_subjectLabel->setObjectName("SubjectLabel");
    m_subjectLabel->setWordWrap(true);
    
    QHBoxLayout *metaLayout = new QHBoxLayout();
    m_senderLabel = new QLabel("发件人: 系统管理员", detailHeader);
    m_senderLabel->setObjectName("MetaLabel");
    m_timeLabel = new QLabel("", detailHeader);
    m_timeLabel->setObjectName("MetaLabel");
    
    // 删除按钮
    m_deleteButton = new QPushButton("删除", detailHeader);
    m_deleteButton->setStyleSheet("color: #EF4444; border-color: #EF4444;");
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setVisible(false); // 初始隐藏
    connect(m_deleteButton, &QPushButton::clicked, this, &SystemEmailDialog::onDeleteEmail);

    metaLayout->addWidget(m_senderLabel);
    metaLayout->addStretch();
    metaLayout->addWidget(m_timeLabel);
    metaLayout->addWidget(m_deleteButton);
    
    headerLayout->addWidget(m_subjectLabel);
    headerLayout->addLayout(metaLayout);
    
    rightLayout->addWidget(detailHeader);

    // 邮件正文
    m_contentViewer = new QTextBrowser(this);
    rightLayout->addWidget(m_contentViewer);

    // 组合布局
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout, 1); // 右侧占据更多空间
}

void SystemEmailDialog::loadEmails()
{
    m_emailList->clear();
    m_emails = DBManager::instance().getSystemEmails(m_account);
    
    int filterIndex = m_filterCombo->currentIndex(); // 0: All, 1: Unread, 2: Read

    for (int i = 0; i < m_emails.size(); ++i) {
        const SystemEmail& email = m_emails[i];
        
        // 筛选逻辑
        if (filterIndex == 1 && email.isRead) continue;
        if (filterIndex == 2 && !email.isRead) continue;

        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::UserRole, email.id); // 存储ID
        
        // 自定义显示文本
        QString statusIcon = email.isRead ? "✅" : "✉️"; 
        QString statusText = email.isRead ? "" : "[未读] ";
        
        QString displayText = QString("%1 %2%3\n   %4")
            .arg(statusIcon)
            .arg(statusText)
            .arg(email.subject)
            .arg(email.createTime.toString("MM-dd HH:mm"));
            
        item->setText(displayText);
        
        // 设置未读加粗
        if (!email.isRead) {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setForeground(QColor("#EF4444")); // 红色
        }
        
        m_emailList->addItem(item);
    }
    
    // 清空右侧
    m_subjectLabel->setText("请选择一封邮件");
    m_timeLabel->setText("");
    m_contentViewer->clear();
    m_deleteButton->setVisible(false);
}

void SystemEmailDialog::onEmailSelected(QListWidgetItem *item)
{
    if (!item) return;
    
    int emailId = item->data(Qt::UserRole).toInt();
    
    // 查找邮件内容
    SystemEmail currentEmail;
    bool found = false;
    for(int i = 0; i < m_emails.size(); ++i) {
        if(m_emails[i].id == emailId) {
            currentEmail = m_emails[i];
            found = true;
            
            // 如果未读，标记为已读
            if (!m_emails[i].isRead) {
                DBManager::instance().markSystemEmailAsRead(emailId);
                m_emails[i].isRead = true;
                
                // 更新列表项显示
                item->setText(QString("✅ %1\n   %2")
                    .arg(currentEmail.subject)
                    .arg(currentEmail.createTime.toString("MM-dd HH:mm")));
                
                QFont font = item->font();
                font.setBold(false);
                item->setFont(font);
                item->setForeground(QColor("#374151"));
            }
            break;
        }
    }
    
    if (!found) return;

    // 更新右侧详情
    updateDetailView(currentEmail);
}

void SystemEmailDialog::updateDetailView(const SystemEmail& email)
{
    m_subjectLabel->setText(email.subject);
    m_timeLabel->setText(email.createTime.toString("yyyy-MM-dd HH:mm:ss"));
    
    // 确保正确处理 UTF-8 编码
    QByteArray bodyBytes = email.body.toUtf8();
    QString bodyText = QString::fromUtf8(bodyBytes);
    m_contentViewer->setPlainText(bodyText);
    
    m_deleteButton->setVisible(true); // 显示删除按钮
}

void SystemEmailDialog::onFilterChanged(int /*index*/)
{
    loadEmails();
}

void SystemEmailDialog::onDeleteEmail()
{
    QListWidgetItem *item = m_emailList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "提示", "请先选择一封邮件");
        return;
    }
    
    int emailId = item->data(Qt::UserRole).toInt();
    qDebug() << "Deleting email, id:" << emailId;
    
    if (QMessageBox::question(this, "确认删除", "确定要删除这封邮件吗？", 
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        bool success = DBManager::instance().deleteSystemEmail(emailId);
        qDebug() << "deleteSystemEmail returned:" << success;
        
        if (success) {
            // 直接从列表中移除该项
            int row = m_emailList->row(item);
            delete m_emailList->takeItem(row);
            
            // 从缓存中移除
            for (int i = 0; i < m_emails.size(); ++i) {
                if (m_emails[i].id == emailId) {
                    m_emails.removeAt(i);
                    break;
                }
            }
            
            // 清空右侧详情
            m_subjectLabel->setText("请选择一封邮件");
            m_timeLabel->setText("");
            m_contentViewer->clear();
            m_deleteButton->setVisible(false);
            
            QMessageBox::information(this, "提示", "删除成功");
        } else {
            QMessageBox::warning(this, "错误", "删除失败，请重试");
        }
    }
}
