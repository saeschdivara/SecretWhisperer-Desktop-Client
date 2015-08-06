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

void ChatController::onError(const QList<QSslError> &errors)
{
    qDebug() << "Error: " << errors;
    qDebug() << "Error String: " << socket->errorString();
}

void ChatController::connectToServer(const QString &url, quint16 port)
{
    // Connections
    connect(socket, &QSslSocket::encrypted, this, &ChatController::connected);

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SIGNAL(error(QAbstractSocket::SocketError)));

    connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(onError(QList<QSslError>)));

    // Finally connect to server
    socket->connectToHostEncrypted(url, port);
}

void ChatController::connectToUser(const QString &username)
{
    qDebug() << "Connecting to user";
    socket->write(QByteArrayLiteral("ddd"));
    socket->waitForBytesWritten();
    qDebug() << "We have written";
}
