#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QString>
#include <QStringList>

class EmailSender : public QObject
{
    Q_OBJECT

public:
    explicit EmailSender(QObject *parent = nullptr);
    ~EmailSender();

    // 发送邮件
    bool sendEmail(const QString &toEmail, const QString &subject, const QString &body);

    // 设置 SMTP 配置
    void setSMTPConfig(const QString &smtpServer, int smtpPort, 
                       const QString &fromEmail, const QString &password, 
                       bool useSSL = true);

private slots:
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

private:
    void sendCommand(const QString &command);
    void parseResponse();
    bool isLastResponse() const;

    enum SMTPState {
        INIT,
        CONNECT,
        AUTH,
        MAIL_FROM,
        MAIL_TO,
        MAIL_DATA,
        MAIL_BODY,
        QUIT,
        DONE
    };

    QSslSocket *m_socket;
    QString m_smtpServer;
    int m_smtpPort;
    QString m_fromEmail;
    QString m_password;
    bool m_useSSL;
    SMTPState m_state;
    QString m_response;
    QString m_toEmail;
    QString m_subject;
    QString m_body;
    bool m_sendingEmail;
    QString m_responseBuffer;
    bool m_authUserSent; // whether we've sent the username during AUTH LOGIN
};

#endif // EMAILSENDER_H
