#include "chatcontroller.h"

#include <QtCore/QFile>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslCipher>

#include <botan/aes.h>
#include <botan/serpent.h>


#include <botan/aes.h>
#include <botan/cryptobox.h>
#include <botan/botan.h>
#include <botan/rsa.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>

const size_t SERPENT_KEY_SIZE = 256;

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
        QByteArray message = messageData.mid(seperatorIndex + seperator.length());

        auto key = connectedUsers.value(username)->symmetricKey();
        QByteArray decryptedMessage = decrypt(message, key);

        qDebug() << "Decrypted message: " << decryptedMessage;

        emit receivedUserMessage(username, decryptedMessage);
    }
    else if ( data.indexOf("STARTUP:") == 0 ) {
        QByteArray messageData = stripRequest(data, "STARTUP:");
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
        QByteArray publicKey = messageData.mid(seperatorIndex + seperator.length());

        ConnectedUser * user;

        // Check if we are already connected to that user
        if ( !connectedUsers.contains(username) ) {
            Botan::DataSource_Memory key_pub(publicKey.toStdString());
            auto publicRsaKey = Botan::X509::load_key(key_pub);

            user = new ConnectedUser(publicRsaKey);
            connectedUsers.insert(username, user);

            qDebug() << "New user with key added";
        }
        else {
            user = connectedUsers.value(username);
        }

        Botan::AutoSeeded_RNG rng;
        Botan::PK_Encryptor_EME encryptor(user->publicKey(), "EME1(SHA-256)");

        Botan::SymmetricKey serpentKey(rng, SERPENT_KEY_SIZE);
        user->setSymmetricKey(serpentKey);

        Botan::byte msgtoencrypt[SERPENT_KEY_SIZE];
        std::string serpentString = serpentKey.as_string();

        for (unsigned int i = 0; i < SERPENT_KEY_SIZE; i++)
        {
            msgtoencrypt[i] = serpentString[i];
        }

        std::vector<Botan::byte> ciphertext = encryptor.encrypt(msgtoencrypt, SERPENT_KEY_SIZE, rng);

        QByteArray keyCipherData;
        keyCipherData.resize(ciphertext.size());

        for ( uint i = 0; i < ciphertext.size(); i++ ) {
            keyCipherData[i] = ciphertext.at(i);
        }

        socket->write(
                    QByteArrayLiteral("ENCRYPT:") +
                    username +
                    seperator +
                    keyCipherData +
                    QByteArrayLiteral("\r\n\r\n")
                    );
    }
    else if ( data.indexOf("ENCRYPT:") == 0 ) {
        QByteArray messageData = stripRequest(data, "ENCRYPT:");
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
        QByteArray message = messageData.mid(seperatorIndex + seperator.length());

        qDebug() << "Received encryption key";

        if ( connectedUsers.contains(username) ) {
            ConnectedUser * user = connectedUsers.value(username);

            qDebug() << "Before decryptor";

            Botan::PK_Decryptor_EME decryptor(user->privateKey(), "EME1(SHA-256)");

            Botan::byte msgToDecrypt[message.length()];

            qDebug() << "Round and roun";

            for (int i = 0; i < message.length(); i++)
            {
                Botan::byte character = (Botan::byte) message.at(i);
                msgToDecrypt[i] = character;
            }

            Botan::secure_vector<Botan::byte> decryptedAesKey = decryptor.decrypt(msgToDecrypt, message.length());
            Botan::SymmetricKey aesKey(decryptedAesKey);

            user->setSymmetricKey(aesKey);
        }
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

    try
        {
            Botan::AutoSeeded_RNG rng;

            Botan::RSA_PrivateKey key(rng, 4096);

            std::string pub = Botan::X509::PEM_encode(key);

            std::string priv = Botan::PKCS8::PEM_encode(key);

            Botan::DataSource_Memory key_pub(pub);

            Botan::DataSource_Memory key_priv(priv);

            auto publicRsaKey = Botan::X509::load_key(key_pub);
            auto privateRsaKey = Botan::PKCS8::load_key(key_priv, rng);

            ConnectedUser * user = new ConnectedUser(publicRsaKey, privateRsaKey, false);
            connectedUsers.insert(username.toUtf8(), user);

            socket->write(
                            QByteArrayLiteral("CONNECT:") +
                            username.toUtf8() +
                            QByteArrayLiteral("\r\n") +
                            QString::fromStdString(pub).toUtf8() +
                            QByteArrayLiteral("\r\n\r\n")
                        );
        }
        catch(std::exception &e)
        {
            qDebug() << e.what();
        }
}

void ChatController::sendMessageToUser(const QString &username, const QString &message)
{
    qDebug() << "Sending user a message";

    auto key = connectedUsers.value(username.toUtf8())->symmetricKey();
    QByteArray encryptedMessage = encrypt(message.toUtf8(), key);

    qDebug() << "Encrypted message: " << encryptedMessage;

    socket->write(
                    QByteArrayLiteral("SEND:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n") +
                    encryptedMessage +
                    QByteArrayLiteral("\r\n\r\n")
                );
}

QByteArray ChatController::encrypt(const QByteArray &input, const Botan::SymmetricKey &key)
{
    const int inputSize = input.length();
    Botan::byte inputData[inputSize];

    for (int i = 0; i < inputSize; ++i) {
        inputData[i] = input.at(i);
    }

    Botan::AutoSeeded_RNG rng;
    std::string encryptedString = Botan::CryptoBox::encrypt(inputData, inputSize, key.as_string(), rng);

    return QByteArray::fromStdString(encryptedString);
}

QByteArray ChatController::decrypt(const QByteArray &input, const Botan::SymmetricKey &key)
{
    const int inputSize = input.length();
    Botan::byte inputData[inputSize];

    for (int i = 0; i < inputSize; ++i) {
        inputData[i] = input.at(i);
    }

    std::string decryptedString = Botan::CryptoBox::decrypt(input.toStdString(), key.as_string());

    return QByteArray::fromStdString(decryptedString);
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
