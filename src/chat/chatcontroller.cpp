#include "chatcontroller.h"

// QT
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QThread>

#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslCipher>

// BOTAN
#include <botan/botan.h>
#include <botan/cryptobox.h>
#include <botan/lzma.h>
#include <botan/pk_keys.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>
#include <botan/rsa.h>
#include <botan/serpent.h>

// SNORTIFY
#include <libsnore/snore.h>
#include <libsnore/notification/notification.h>
#include <libsnore/log.h>

using namespace Snore;

const size_t SERPENT_KEY_SIZE = 128;

/**
 * @brief ChatController::ChatController
 * @param parent
 */
ChatController::ChatController(QObject *parent) : QObject(parent),
    core(SnoreCore::instance()),
    icon(QString(":/root/snore.png")),
    snoreApplication(Application(qApp->applicationName(), icon)),
    alert(Alert(QString("Default"), icon))
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

    // Snortify
    Snore::SnoreLog::setDebugLvl(1);

    //Get the core
    Snore::SnoreCore::instance().loadPlugins(
                Snore::SnorePlugin::BACKEND | Snore::SnorePlugin::SECONDARY_BACKEND
    );

    //All notifications have to have icon, so prebuild one
    core.registerApplication(snoreApplication);

    //Also alert is mandatory, just choose the default one
    snoreApplication.addAlert(alert);
}

ChatController::~ChatController()
{
    qDebug() << "Killed controller";
    socket->close();
    socket->disconnectFromHost();
}

/**
 * @brief ChatController::onConnectionEstablished
 */
void ChatController::onConnectionEstablished()
{
    //
}

/**
 * @brief ChatController::onServerData
 */
void ChatController::onServerData()
{
    disconnect( socket, &QTcpSocket::readyRead, this, &ChatController::onServerData );

    QByteArray data, tempData;

    qDebug() << "Receiving data";

    while ( !socket->atEnd() || !data.contains("\r\n\r\n") ) {
        tempData = socket->read(1024);

        if ( tempData.size() > 0 ) {
            data.append(tempData);
        }
        else {
            QThread::currentThread()->msleep(100);
        }

        qApp->processEvents();
    }

    connect( socket, &QTcpSocket::readyRead, this, &ChatController::onServerData );

    qDebug() << "Received server data";

    if ( data.indexOf("MESSAGE:") == 0 ) {
        onMessageEvent(data);
    }
    else if ( data.indexOf("STARTUP:") == 0 ) {
        onStartupEvent(data);
    }
    else if ( data.indexOf("ENCRYPT:") == 0 ) {
        onEncryptEvent(data);
    }
    else {
        qDebug() << "Unknwon: " << data;
    }
}

/**
 * @brief ChatController::onError
 * @param errors
 */
void ChatController::onError(const QList<QSslError> &errors)
{
    qDebug() << "Error: " << errors;
    qDebug() << "Error String: " << socket->errorString();
}

/**
 * @brief ChatController::connectToServer
 * @param url
 * @param port
 */
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

/**
 * @brief ChatController::chooseUserName
 * @param username
 */
void ChatController::chooseUserName(const QString &username)
{
    qDebug() << "Choosing username";
    socket->write(
                    QByteArrayLiteral("USER:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n\r\n")
                );
}

/**
 * @brief ChatController::connectToUser
 * @param username
 */
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

            emit connectionToUserEstablished(username);

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

/**
 * @brief ChatController::sendMessageToUser
 * @param username
 * @param message
 */
void ChatController::sendMessageToUser(const QString &username, const QString &message)
{
    qDebug() << "Sending user a message";

    if ( !connectedUsers.contains(username.toUtf8()) ) {
        qDebug() << "You are not connected to this user";
        return;
    }

    auto key = connectedUsers.value(username.toUtf8())->symmetricKey();
    QByteArray encryptedMessage = encrypt(message.toUtf8(), key);

    qDebug() << "Encrypted message: " << encryptedMessage;

    socket->write(
                    QByteArrayLiteral("SEND:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n")
                );

    qDebug() << "Send data";
    socket->waitForBytesWritten();
    qDebug() << "Data written";

    sendSplittedData(encryptedMessage, 1024);
}

/**
 * @brief ChatController::onStartupEvent
 * @param data
 */
void ChatController::onStartupEvent(const QByteArray &data)
{
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

    emit connectionToUserEstablished(username);

    Botan::AutoSeeded_RNG rng;
    Botan::PK_Encryptor_EME encryptor(user->publicKey(), "EME1(SHA-256)");

    Botan::SymmetricKey serpentKey(rng, SERPENT_KEY_SIZE);
    user->setSymmetricKey(serpentKey);

    std::string serpentString = serpentKey.as_string();
    const int HEX_KEY_SIZE = serpentString.size();
    Botan::byte msgtoencrypt[HEX_KEY_SIZE];

    for (unsigned int i = 0; i < HEX_KEY_SIZE; i++)
    {
        msgtoencrypt[i] = serpentString[i];
    }

    std::vector<Botan::byte> ciphertext = encryptor.encrypt(msgtoencrypt, HEX_KEY_SIZE, rng);

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

/**
 * @brief ChatController::onEncryptEvent
 * @param data
 */
void ChatController::onEncryptEvent(const QByteArray &data)
{
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

        // Decrypt key with private key
        Botan::secure_vector<Botan::byte> decryptedSerpentKey = decryptor.decrypt(msgToDecrypt, message.length());

        // The key is transfered in hex
        QByteArray hexKey;

        for (int i = 0; i < decryptedSerpentKey.size(); ++i) {
            Botan::byte dataByte = decryptedSerpentKey[i];
            hexKey.append((char) dataByte);
        }

        Botan::SymmetricKey aesKey(hexKey.toStdString());

        user->setSymmetricKey(aesKey);
    }
}

/**
 * @brief ChatController::onMessageEvent
 * @param data
 */
void ChatController::onMessageEvent(const QByteArray &data)
{
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

    // Inform the user of the new message
    Notification n(snoreApplication, alert,
                   QStringLiteral("Message from ") + username,
                   decryptedMessage,
                   icon);

    // Optional: you can also set delivery date if you want to schedule notification
    //n.setDeliveryDate(QDateTime::currentDateTime().addSecs(5));

    core.broadcastNotification(n);
}

void ChatController::sendSplittedData(QByteArray &data, quint64 max_piece_size)
{
    quint64 data_size = data.size();

    qDebug() << "Started sending";

    while ( data_size > 0 ) {

        if ( data_size >= max_piece_size ) {
            data_size -= max_piece_size;

            QByteArray tempData = data.left(max_piece_size);
            socket->write(tempData);
            socket->waitForBytesWritten();
            qDebug() << "Data written";
            data = data.remove(0, max_piece_size);
        }
        else {
            data_size = 0;
            socket->write(data + QByteArrayLiteral("\r\n\r\n"));
            socket->waitForBytesWritten();
            data.clear();
        }

    }

    qDebug() << "Finished sending";
}

/**
 * @brief ChatController::encrypt
 * @param input
 * @param key
 * @return
 */
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

/**
 * @brief ChatController::decrypt
 * @param input
 * @param key
 * @return
 */
QByteArray ChatController::decrypt(const QByteArray &input, const Botan::SymmetricKey &key)
{
    std::string decryptedString = Botan::CryptoBox::decrypt(input.toStdString(), key.as_string());
    return QByteArray::fromStdString(decryptedString);
}

/**
 * @brief ChatController::stripRequest
 * @param data
 * @param command
 * @return
 */
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
