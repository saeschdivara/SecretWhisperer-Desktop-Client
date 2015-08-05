#include "chatcontroller.h"

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCipher>

#include <botan/aes.h>
#include <botan/serpent.h>

ChatController::ChatController(QObject *parent) : QObject(parent)
{
    socket = new QSslSocket(this);

    // Ciphers allowed
    QList<QSslCipher> ciphers;

    ciphers << QSslCipher("ECDHE-RSA-AES-128-CBC-SHA256");

    // Update config
    QSslConfiguration config = socket->sslConfiguration();
    config.setProtocol(QSsl::TlsV1_2);

    // Set allowed ciphers
    config.setCiphers(ciphers);

    socket->setSslConfiguration(config);
}

void ChatController::connectToServer(const QString &url, quint16 port)
{
    // Connections
    connect(socket, &QSslSocket::connected, this, &ChatController::connected);

    // Finally connect to server
    socket->connectToHostEncrypted(url, port);
}
