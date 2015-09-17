#include "chatcontroller.h"

// QT
#include <QtCore/QFile>
#include <QtCore/QThread>

// BOTAN
#include <botan/serpent.h>
#include <botan/hmac.h>
#include <botan/sha160.h>
#include <botan/dlies.h>

/**
 * @brief ChatController::ChatController
 * @param parent
 */
ChatController::ChatController(QObject *parent) : QObject(parent),
    // Connection objects
    socket(new QSslSocket(this)),
    connector(new Connector(socket, this)),
    notifyer(new NotificationController(this)),
    protocol(new ProtocolController(this)),
    identity(new IdentityController(this))
{
    protocol->prepareConnection(socket);
}

ChatController::~ChatController()
{
    qDebug() << "Killed controller";
    socket->close();
    socket->disconnectFromHost();
}

void ChatController::listenOnErrors()
{
    // Errors
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SIGNAL(error(QAbstractSocket::SocketError)));

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));

    connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(onError(QList<QSslError>)));
}

void ChatController::listenOnProtocol()
{
    connect(connector, &Connector::newData, protocol, &ProtocolController::onServerDataEvent );

    // Protocol signals
    connect( protocol, &ProtocolController::signalStartup, this, &ChatController::onStartupEvent );
    connect( protocol, &ProtocolController::signalEncrypt, this, &ChatController::onEncryptEvent );
    connect( protocol, &ProtocolController::signalMessage, this, &ChatController::onMessageEvent );
}

/**
 * @brief ChatController::onConnectionEstablished
 */
void ChatController::onConnectionEstablished()
{

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

void ChatController::onSocketError(QAbstractSocket::SocketError error)
{
    qWarning() << "Error: " << error;
    qWarning() << "Error String: " << socket->errorString();
}

/**
 * @brief ChatController::connectToServer
 * @param url
 * @param port
 */
void ChatController::connectToServer(const QString &url, quint16 port)
{
    // Listen on start
    connect(socket, &QSslSocket::encrypted, this, &ChatController::connected);

    listenOnErrors();
    listenOnProtocol();

    // Connections
    connector->listen();

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
    connector->onMessage(QByteArray("USER:"), username.toUtf8());
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
        // Add user to internal user list
        ConnectedUser * user = protocol->createUser();
        connectedUsers.insert(username.toUtf8(), user);

        QByteArray pub = user->getDataFromPublicKey();

        emit connectionToUserEstablished(username);

        // Send public key to the other user
        connector->onMessage(QByteArray("CONNECT:"), username.toUtf8(), pub);
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

    ConnectedUser * user = connectedUsers.value(username.toUtf8());
    QByteArray messageData = message.toUtf8();
    QByteArray encryptedMessage = protocol->encryptWithSymmetricKey(user, messageData);

    qDebug() << "Encrypted message: " << encryptedMessage;
    connector->onMessage(QByteArray("SEND:"), username.toUtf8(), encryptedMessage);
}

/**
 * @brief ChatController::onStartupEvent
 * @param data
 */
void ChatController::onStartupEvent(const QByteArray &username, QByteArray & publicKey)
{
    ConnectedUser * user;

    // Check if we are already connected to that user
    if ( !connectedUsers.contains(username) ) {
        user = protocol->createUserFromPublicKey(publicKey);
        connectedUsers.insert(username, user);

        qDebug() << "New user with key added";
    }
    else {
        user = connectedUsers.value(username);
    }

    emit connectionToUserEstablished(username);

    protocol->createSymmetricKeyForUser(user);

    std::string serpentString = user->symmetricKey().as_string();
    QByteArray keyCipherData = protocol->encryptWithAsymmetricKey(user, serpentString);

    connector->onMessage(QByteArray("ENCRYPT:"), username, keyCipherData);
}

/**
 * @brief ChatController::onEncryptEvent
 * @param data
 */
void ChatController::onEncryptEvent(const QByteArray &username, QByteArray & message)
{
    qDebug() << "Received encryption key";

    if ( connectedUsers.contains(username) ) {
        ConnectedUser * user = connectedUsers.value(username);

        // The key is transfered in hex
        QByteArray hexKey = protocol->decryptWithAsymmetricKey(user, message);
        Botan::SymmetricKey key(hexKey.toStdString());

        user->setSymmetricKey(key);
    }
}

/**
 * @brief ChatController::onMessageEvent
 * @param data
 */
void ChatController::onMessageEvent(const QByteArray &username, QByteArray & message)
{
    ConnectedUser * user = connectedUsers.value(username);
    QByteArray decryptedMessage = protocol->decryptWithSymmetricKey(user, message);

    emit receivedUserMessage(username, decryptedMessage);

    notifyer->showNotification(QStringLiteral("Message from ") + username, decryptedMessage);
}
