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
    const QByteArray seperator("\r\n");

    if ( data.indexOf("MESSAGE:") == 0 ) {
        QByteArray preparedData = prepareRequest(data, QByteArrayLiteral("MESSAGE:"));

        if ( preparedData.size() > 0 ) {

            int seperatorIndex = preparedData.indexOf(seperator);

            QByteArray username = preparedData.left(seperatorIndex);
            QByteArray message = preparedData.mid(seperatorIndex + seperator.length());

            signalMessage(username, message);
        }
    }
    else if ( data.indexOf("STARTUP:") == 0 ) {
        QByteArray preparedData = prepareRequest(data, QByteArrayLiteral("STARTUP:"));

        if ( preparedData.size() > 0 ) {

            int seperatorIndex = preparedData.indexOf(seperator);

            QByteArray username = preparedData.left(seperatorIndex);
            QByteArray publicKey = preparedData.mid(seperatorIndex + seperator.length());

            signalStartup(username, publicKey);
        }
    }
    else if ( data.indexOf("ENCRYPT:") == 0 ) {
        QByteArray preparedData = prepareRequest(data, QByteArrayLiteral("ENCRYPT:"));

        if ( preparedData.size() > 0 ) {

            int seperatorIndex = preparedData.indexOf(seperator);

            QByteArray username = preparedData.left(seperatorIndex);
            QByteArray message = preparedData.mid(seperatorIndex + seperator.length());

            signalEncrypt(username, message);
        }
    }
    else {
        signalError(QByteArrayLiteral("UNKNOWN ACTION"));
    }
}

QByteArray ProtocolController::prepareRequest(QByteArray &data, QByteArray command)
{
    QByteArray messageData = stripRequest(data, command);
    const QByteArray seperator("\r\n");

    int seperatorIndex = messageData.indexOf(seperator);

    if ( seperatorIndex == -1 ) {
        signalError(QByteArrayLiteral("MISSING SEPERATOR"));
        return QByteArray();
    }

    if ( seperatorIndex == 0 ) {
        signalError(QByteArrayLiteral("MISSING USER"));
        return QByteArray();
    }

    return messageData;
}

QByteArray ProtocolController::stripRequest(QByteArray data, QByteArray command)
{
    const QByteArray endLiteral("\r\n\r\n");

    if ( data.indexOf(command) != 0 ) {
        signalError(QByteArrayLiteral("UNKNOWN COMMAND"));
        return QByteArray();
    }

    data = data.remove(0, command.length());

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        signalError(QByteArrayLiteral("NO END"));
        return QByteArray();
    }

    // Get user name
    data = data.remove(endIndex, endLiteral.length());

    return data;
}

