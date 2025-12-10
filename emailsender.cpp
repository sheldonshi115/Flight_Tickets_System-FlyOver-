#include "emailsender.h"
#include <QDebug>
#include <QDateTime>
#include <QTimer>

EmailSender::EmailSender(QObject *parent)
    : QObject(parent), 
      m_socket(nullptr),
      m_smtpPort(465),
      m_useSSL(true),
      m_state(INIT),
      m_sendingEmail(false)
{
    m_socket = new QSslSocket(this);
    connect(m_socket, &QSslSocket::connected, this, &EmailSender::onConnected);
    connect(m_socket, &QSslSocket::readyRead, this, &EmailSender::onReadyRead);
        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &EmailSender::onError);
    connect(m_socket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            this, &EmailSender::onSslErrors);
    m_authUserSent = false;
}

EmailSender::~EmailSender()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void EmailSender::setSMTPConfig(const QString &smtpServer, int smtpPort,
                                 const QString &fromEmail, const QString &password,
                                 bool useSSL)
{
    m_smtpServer = smtpServer;
    m_smtpPort = smtpPort;
    m_fromEmail = fromEmail;
    m_password = password;
    m_useSSL = useSSL;
}

bool EmailSender::sendEmail(const QString &toEmail, const QString &subject, const QString &body)
{
    if (m_sendingEmail) {
        qWarning() << "Already sending an email, please wait";
        return false;
    }

    if (m_smtpServer.isEmpty() || m_fromEmail.isEmpty() || m_password.isEmpty()) {
        qWarning() << "SMTP configuration not set";
        return false;
    }

    m_toEmail = toEmail;
    m_subject = subject;
    m_body = body;
    m_sendingEmail = true;
    m_state = CONNECT;
    m_responseBuffer.clear();
    m_authUserSent = false;

    qDebug() << "Connecting to SMTP server:" << m_smtpServer << ":" << m_smtpPort;

    if (m_useSSL) {
        m_socket->connectToHostEncrypted(m_smtpServer, m_smtpPort);
    } else {
        m_socket->connectToHost(m_smtpServer, m_smtpPort);
    }

    return true;
}

void EmailSender::onConnected()
{
    qDebug() << "Connected to SMTP server";
    m_state = INIT;
    // Wait for server greeting
}

void EmailSender::onReadyRead()
{
    m_responseBuffer += m_socket->readAll();

    // Process complete lines
    while (m_responseBuffer.contains("\r\n")) {
        int endPos = m_responseBuffer.indexOf("\r\n");
        QString line = m_responseBuffer.left(endPos);
        m_responseBuffer = m_responseBuffer.mid(endPos + 2);

        qDebug() << "SMTP Response:" << line;

        // Check if this is the last line of a response (response code followed by space or dash)
        if (line.length() >= 4) {
            QString code = line.left(3);
            QChar separator = line[3];

            // Process the response based on current state
            if (separator != '-') { // Not a multi-line response
                if (m_state == INIT && code == "220") {
                    m_state = AUTH;
                    sendCommand("EHLO localhost");
                } else if (m_state == AUTH && (code == "250" || code == "220")) {
                    // Send AUTH LOGIN
                    sendCommand("AUTH LOGIN");
                } else if (m_state == AUTH && code == "334") {
                    // Server is prompting for AUTH LOGIN (first 334 for username, second 334 for password)
                    if (!m_authUserSent) {
                        QByteArray username = m_fromEmail.toLatin1();
                        QString encoded = QString::fromLatin1(username.toBase64());
                        sendCommand(encoded);
                        m_authUserSent = true;
                    } else {
                        QByteArray password = m_password.toLatin1();
                        QString encoded = QString::fromLatin1(password.toBase64());
                        sendCommand(encoded);
                    }
                } else if (m_state == AUTH && code == "235") {
                    // Authentication successful
                    m_state = MAIL_FROM;
                    sendCommand("MAIL FROM:<" + m_fromEmail + ">");
                } else if (m_state == MAIL_FROM && code == "250") {
                    m_state = MAIL_TO;
                    sendCommand("RCPT TO:<" + m_toEmail + ">");
                } else if (m_state == MAIL_TO && code == "250") {
                    m_state = MAIL_DATA;
                    sendCommand("DATA");
                } else if (m_state == MAIL_DATA && code == "354") {
                    // Now send the email body
                    // Build RFC-compliant headers
                    // From: <email@domain>
                    QString headers = "From: <" + m_fromEmail + ">\r\n";
                    headers += "To: <" + m_toEmail + ">\r\n";
                    headers += "Subject: =?UTF-8?B?" + QString::fromLatin1(m_subject.toUtf8().toBase64()) + "?=\r\n";
                    headers += "MIME-Version: 1.0\r\n";
                    headers += "Content-Type: text/plain; charset=utf-8\r\n";
                    headers += "Content-Transfer-Encoding: base64\r\n";
                    headers += "Date: " + QDateTime::currentDateTimeUtc().toString("ddd, dd MMM yyyy HH:mm:ss +0000") + "\r\n";
                    headers += "Message-ID: <" + QByteArray::number(QDateTime::currentMSecsSinceEpoch()).toHex() + "@local>\r\n";
                    headers += "\r\n";

                    // Encode body as base64 for UTF-8 support
                    QString encodedBody = QString::fromLatin1(m_body.toUtf8().toBase64());
                    QString fullMessage = headers + encodedBody + "\r\n.\r\n";
                    m_socket->write(fullMessage.toLatin1());
                    m_state = MAIL_BODY;
                } else if (m_state == MAIL_BODY && code == "250") {
                    // Email sent successfully
                    m_state = QUIT;
                    sendCommand("QUIT");
                } else if (m_state == QUIT && code == "221") {
                    // Goodbye from server
                    m_socket->disconnectFromHost();
                    m_sendingEmail = false;
                    qDebug() << "Email sent successfully";
                } else if (code.at(0).isDigit() && code.at(0) >= '4') {
                    // Error response
                    qWarning() << "SMTP Error:" << line;
                    m_socket->disconnectFromHost();
                    m_sendingEmail = false;
                }
            }
        }
    }
}

void EmailSender::onError(QAbstractSocket::SocketError error)
{
    qWarning() << "Socket error:" << error << m_socket->errorString();
    m_sendingEmail = false;
}

void EmailSender::onSslErrors(const QList<QSslError> &errors)
{
    qWarning() << "SSL errors:";
    for (const auto &error : errors) {
        qWarning() << error.errorString();
    }
    // Ignore SSL errors for self-signed certificates
    m_socket->ignoreSslErrors();
}

void EmailSender::sendCommand(const QString &command)
{
    qDebug() << "Sending:" << command;
    m_socket->write((command + "\r\n").toLatin1());
}
