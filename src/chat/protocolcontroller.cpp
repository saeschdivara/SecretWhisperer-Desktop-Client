#include "protocolcontroller.h"

#include <QtCore/QFile>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslCipher>

ProtocolController::ProtocolController(QObject *parent) : QObject(parent)
{
    encryptor = new Encryptor(this);
}

void ProtocolController::prepareConnection(QSslSocket *socket)
{
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

ConnectedUser *ProtocolController::createUser()
{
    KeyPair pair = encryptor->createAsymmetricKeys();
    return new ConnectedUser(pair.publicKey, pair.privateKey, false);
}

void ProtocolController::onServerDataEvent(QByteArray &data)
{
    //
}

