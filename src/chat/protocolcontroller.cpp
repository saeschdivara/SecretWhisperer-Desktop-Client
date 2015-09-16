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

ConnectedUser *ProtocolController::createUserFromPublicKey(QByteArray &publicKey)
{
    Botan::DataSource_Memory key_pub(publicKey.toStdString());
    auto publicRsaKey = Botan::X509::load_key(key_pub);

    return new ConnectedUser(publicRsaKey);
}

void ProtocolController::createSymmetricKeyForUser(ConnectedUser *user)
{
    Botan::SymmetricKey key = encryptor->createSymmetricKey();
    user->setSymmetricKey(key);
}

QByteArray ProtocolController::encryptWithAsymmetricKey(ConnectedUser *user, std::string &data)
{
    return encryptor->encryptAsymmetricly(user, data);
}

QByteArray ProtocolController::decryptWithAsymmetricKey(ConnectedUser *user, QByteArray &data)
{
    return encryptor->decryptAsymmetricly(user, data);
}

QByteArray ProtocolController::encryptWithSymmetricKey(ConnectedUser *user, QByteArray &data)
{
    return encryptor->encryptSymmetricly(user, data);
}

QByteArray ProtocolController::decryptWithSymmetricKey(ConnectedUser *user, QByteArray &data)
{
    return encryptor->decryptSymmetricly(user, data);
}

void ProtocolController::onServerDataEvent(QByteArray &data)
{
    //
}

