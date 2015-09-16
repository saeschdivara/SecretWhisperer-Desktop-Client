#include "chatcontroller.h"

// QT
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QThread>

// BOTAN
#include <botan/serpent.h>
#include <botan/hmac.h>
#include <botan/sha160.h>
#include <botan/dlies.h>

// SNORTIFY
#include <libsnore/snore.h>
#include <libsnore/notification/notification.h>
#include <libsnore/log.h>

using namespace Snore;

/**
 * @brief ChatController::ChatController
 * @param parent
 */
ChatController::ChatController(QObject *parent) : QObject(parent),
    core(SnoreCore::instance()),
    icon(QString(":/root/snore.png")),
    snoreApplication(Application(qApp->applicationName(), icon)),
    alert(Alert(QString("Default"), icon)),

    // Connection objects
    socket(new QSslSocket(this)),
    connector(new Connector(socket, this)),
    protocol(new ProtocolController(this))
{
    protocol->prepareConnection(socket);

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
void ChatController::onServerData(QByteArray & data)
{

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
    // Connections
    connect(socket, &QSslSocket::encrypted, this, &ChatController::connected);
    connect(connector, &Connector::newData, this, &ChatController::onServerData );

    connector->listen();

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SIGNAL(error(QAbstractSocket::SocketError)));

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onSocketError(QAbstractSocket::SocketError)));

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
    connector->send(
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
            // Add user to internal user list
            ConnectedUser * user = protocol->createUser();
            connectedUsers.insert(username.toUtf8(), user);

            QByteArray pub = user->getDataFromPublicKey();

            emit connectionToUserEstablished(username);

            // Send public key to the other user
            connector->send(
                            QByteArrayLiteral("CONNECT:") +
                            username.toUtf8() +
                            QByteArrayLiteral("\r\n") +
                            pub +
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

    ConnectedUser * user = connectedUsers.value(username.toUtf8());
    QByteArray messageData = message.toUtf8();
    QByteArray encryptedMessage = protocol->encryptWithSymmetricKey(user, messageData);

    qDebug() << "Encrypted message: " << encryptedMessage;

    connector->send(
                    QByteArrayLiteral("SEND:") +
                    username.toUtf8() +
                    QByteArrayLiteral("\r\n") +
                    encryptedMessage +
                    QByteArrayLiteral("\r\n\r\n")
                );
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
        connector->send(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
        return;
    }

    if ( seperatorIndex == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
        return;
    }

    QByteArray username = messageData.left(seperatorIndex);
    QByteArray publicKey = messageData.mid(seperatorIndex + seperator.length());

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

    connector->send(
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
        connector->send(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
        return;
    }

    if ( seperatorIndex == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
        return;
    }

    QByteArray username = messageData.left(seperatorIndex);
    QByteArray message = messageData.mid(seperatorIndex + seperator.length());

    qDebug() << "Received encryption key";

    if ( connectedUsers.contains(username) ) {
        ConnectedUser * user = connectedUsers.value(username);

        // The key is transfered in hex
        QByteArray hexKey = protocol->decryptWithAsymmetricKey(user, message);
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
        connector->send(QByteArrayLiteral("ERROR:MISSING SEPERATOR\r\n\r\n"));
        return;
    }

    if ( seperatorIndex == 0 ) {
        connector->send(QByteArrayLiteral("ERROR:MISSING USER\r\n\r\n"));
        return;
    }

    QByteArray username = messageData.left(seperatorIndex);
    QByteArray message = messageData.mid(seperatorIndex + seperator.length());

    ConnectedUser * user = connectedUsers.value(username);
    QByteArray decryptedMessage = protocol->decryptWithSymmetricKey(user, message);
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
        connector->send(QByteArrayLiteral("ERROR:UNKNOWN COMMAND\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    data = data.remove(0, command.length());

    int endIndex = data.indexOf(endLiteral);
    if ( endIndex == -1 ) {
        connector->send(QByteArrayLiteral("ERROR:NO END\r\n\r\n"));
        socket->close();
        return QByteArrayLiteral("");
    }

    // Get user name
    data = data.remove(endIndex, endLiteral.length());

    return data;
}
