#include "chatcontroller.h"

#include <QtCore/QFile>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslCipher>

#include <botan/aes.h>
#include <botan/serpent.h>

ChatController::ChatController(QObject *parent) : QObject(parent)
{
    socket = new QSslSocket(this);

    // Ciphers allowed
    QList<QSslCipher> ciphers;

    ciphers << QSslCipher("ECDHE-RSA-AES256-GCM-SHA384");

    // Update config
    QSslConfiguration config = socket->sslConfiguration();
    config.setProtocol(QSsl::TlsV1_2);

    // Set allowed ciphers
    config.setCiphers(ciphers);

    socket->setSslConfiguration(config);

    QFile caFile("certificates/cert.pem");
    caFile.open(QIODevice::ReadOnly);

    QByteArray data = caFile.readAll();

    caFile.close();

    QSslCertificate certifacte(data);
    socket->addCaCertificate(certifacte);
}

ChatController::~ChatController()
{
    qDebug() << "Killed controller";
    socket->disconnectFromHost();
}

void ChatController::onConnectionEstablished()
{
    //
}

void ChatController::onServerData()
{
    QByteArray data = socket->readAll();

    qDebug() << "Received server data";

    if ( data.indexOf("MESSAGE:") == 0 ) {
        QByteArray messageData = stripRequest(data, "MESSAGE:");
        const QByteArray seperator("\r\n");

        int seperatorIndex = messageData.indexOf(seperator);

        if ( seperatorIndex == -1 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
            return;
        }

        if ( seperatorIndex == 0 ) {
            socket->write(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
            return;
        }

        QByteArray username = messageData.left(seperatorIndex);
        QByteArray message = messageData.right(seperatorIndex);

        qDebug() << "Username: " << username;
        qDebug() << "Message: " << message;
    }
    else {
        qDebug() << "Unknwon: " << data;
    }
}

void ChatController::onError(const QList<QSslError> &errors)
{
    qDebug() << "Error: " << errors;
    qDebug() << "Error String: " << socket->errorString();
}

void ChatController::connectToServer(const QString &url, quint16 port)
{
    // Connections
    connect(socket, &QSslSocket::encrypted, this, &ChatController::connected);
    connect(socket, &QSslSocket::readyRead, this, &ChatController::onServerData);

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SIGNAL(error(QAbstractSocket::SocketError)));

    connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(onError(QList<QSslError>)));

    // Finally connect to server
    socket->connectToHostEncrypted(url, port);
}

void ChatController::chooseUserName(const QString &username)
{
    qDebug() << "Choosing username";
    socket->write(
                    QByteArrayLiteral("USER:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n\r\n")
                );
}

void ChatController::connectToUser(const QString &username)
{
    qDebug() << "Connecting to user";
    socket->write(
                    QByteArrayLiteral("CONNECT:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n\r\n")
                );
}

void ChatController::sendMessageToUser(const QString &username, const QString &message)
{
    qDebug() << "Sending user a message";
    socket->write(
                    QByteArrayLiteral("SEND:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n") +
                    message.toUtf8() +
                    QByteArrayLiteral("\r\n\r\n")
                );
}

QByteArray ChatController::stripRequest(QByteArray data, QByteArray command)
{
    const QByteArray endLiteral("\r\n\r\n");

    if ( data.indexOf(command) != 0 ) {
        socket->write(QByteArrayLiteral("ERROR:UNKNOWN COMMAND\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    data = data.remove(0, command.length());

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        socket->write(QByteArrayLiteral("ERROR:NO END\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    // Get user name
    data = data.remove(endIndex, endLiteral.length());

    return data;
}
