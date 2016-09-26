#include "chatcontroller.h"

// QT
#include <QtGui/QDesktopServices>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMimeDatabase>
#include <QtCore/QMimeType>
#include <QtCore/QThread>
#include <QtCore/QUrl>

// BOTAN
#include <botan/serpent.h>
#include <botan/hmac.h>
#include <botan/sha160.h>
#include <botan/dlies.h>

const QString attachmentFolderName("attachmentes");

QByteArray createFileFromDataUri(QByteArray & dataUri);

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
    connect( protocol, &ProtocolController::signalAuthenticationSucceded, this, &ChatController::onAuthenticationSucceded );
    connect( protocol, &ProtocolController::signalStartup, this, &ChatController::onStartupEvent );
    connect( protocol, &ProtocolController::signalIdentityCheck, this, &ChatController::onIdentityCheckEvent );
    connect( protocol, &ProtocolController::signalEncrypt, this, &ChatController::onEncryptEvent );
    connect( protocol, &ProtocolController::signalMessage, this, &ChatController::onMessageEvent );
    connect( protocol, &ProtocolController::signalUserOnline, this, &ChatController::connectToUser );
}

void ChatController::onLinkClicked(const QUrl &url)
{
    qDebug() << "Link clicked: " << url;
    QString dataPath("stream://data.local/");
    QString path(url.path());

    if ( path.startsWith(dataPath) ) {
        path = path.replace(dataPath, "");

        QDir currentPath = QDir::current();
        QString filePath = currentPath.absoluteFilePath(path);
        QDesktopServices::openUrl(QUrl(filePath));
    }
}

/**
 * @brief ChatController::loadContacts
 */
void ChatController::loadContacts()
{
    contacts = identity->getContacts();
    qDebug() << "Contacts loaded: " << contacts.keys();

    emit contactsLoaded(contacts.keys().join("|"));
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
void ChatController::chooseUserName(const QString &username, const QString &password)
{
    qDebug() << "Choosing username";

    identity->createUserIdentity(username, password);

    connector->onMessage(QByteArray("USER:"),
                         username.toUtf8(),
                         QByteArray::fromStdString(identity->getSymmetricKeyString()));

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

        QByteArray publicKey = user->getDataFromPublicKey();

        emit connectionToUserEstablished(username);

        // Create contact
        if (!contacts.contains(username)) {
            identity->addContact(username, publicKey);
            contacts.insert(username, user);
        }

        // Send public key to the other user
        connector->onMessage(QByteArray("CONNECT:"), username.toUtf8(), publicKey);
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

    connector->onMessage(QByteArray("SEND:"), username.toUtf8(), encryptedMessage);
}

void ChatController::onAuthenticationSucceded()
{
    qDebug() << "Authentication succeded";

    for ( QString contactName : contacts.keys() ) {
        connector->onMessage(QByteArrayLiteral("NOTIFY-USER-ONLINE:"), contactName.toUtf8());
    }
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

    // Create contact
    if (!contacts.contains(username)) {
        identity->addContact(username, publicKey);
        contacts.insert(username, user);
    }

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

    if ( decryptedMessage.startsWith("unsafe:data:") || decryptedMessage.startsWith("data:") ) {
        QByteArray fileUrl = createFileFromDataUri(decryptedMessage);
        emit receivedUserFile(username, fileUrl);
    }
    else {
        emit receivedUserMessage(username, decryptedMessage);
        if (decryptedMessage.size() < 200) {
            notifyer->showNotification(QStringLiteral("Message from ") + username, decryptedMessage);
        }
    }

}

/**
 * @brief ChatController::onIdentityCheckEvent
 * @param encryptedRandomString
 */
void ChatController::onIdentityCheckEvent(QByteArray &encryptedRandomString)
{
    QByteArray randomString = protocol->decryptWithAsymmetricKey(identity->getUser(), encryptedRandomString);
    QByteArray randomStringHash = QCryptographicHash::hash(randomString, QCryptographicHash::Sha3_512);

    connector->onMessage(QByteArray("AUTHENTICATE:"), randomStringHash);
}

// --------------------------------------------------------------------------------------------
// HELPER FUNCTIONS
// --------------------------------------------------------------------------------------------

/**
 * @brief getOrCreate
 * @param baseDir
 * @param dirName
 * @return
 */
QDir getOrCreate(QDir baseDir, QString dirName) {
    QDir newDir(baseDir.absolutePath() + QDir::separator() + dirName);

    if ( !newDir.exists() ) {
        baseDir.mkdir(dirName);
    }

    return newDir;
}

/**
 * @brief getFilePath
 * @param baseDir
 * @param fileName
 * @param suffix
 * @return
 */
QString getFilePath(QDir baseDir, QString fileName, QString suffix) {
    return baseDir.absoluteFilePath(fileName + QChar('.') + suffix);
}

/**
 * @brief createFileFromDataUri
 * @param dataUri
 * @return
 */
QByteArray createFileFromDataUri(QByteArray & dataUri) {
    QString path;

    QDir currentDirectory = QDir::current();

    if ( !currentDirectory.cd(attachmentFolderName) ) {
        // Create attachements folder
        currentDirectory.mkdir(attachmentFolderName);
        currentDirectory.cd(attachmentFolderName);
    }

    uint index = 0;
    QDir attachmentsSubFolder = getOrCreate(currentDirectory, QString::number(index));
    while ( attachmentsSubFolder.count() > 255 ) {
        index++;
        attachmentsSubFolder =
                getOrCreate(currentDirectory, QString::number(index));
    }

    if ( dataUri.startsWith("data:") ) {
        // Remove unused part
        dataUri = dataUri.remove(0, QString("data:").length());
    }
    else if ( dataUri.startsWith("unsafe:data:") ) {
        // Remove unused part
        dataUri = dataUri.remove(0, QString("unsafe:data:").length());
    }

    // data:image/png;base64,
    QByteArrayList splittedDataUri = dataUri.split(',');
    QByteArray data = QByteArray::fromBase64(splittedDataUri.at(1));

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForData(data);
    QString suffix = mime.preferredSuffix();

    if ( suffix.isEmpty() ) {
        suffix = "unknown";
    }

    index = 0;
    QFile dataFile;

    do {
        path = getFilePath(attachmentsSubFolder, QString::number(index), suffix);
        dataFile.setFileName(path);
        index++;
    } while (dataFile.exists());

    dataFile.open(QIODevice::WriteOnly);
    dataFile.write(data);
    dataFile.close();

    int indexAttachmentsFolder = path.indexOf(attachmentFolderName);
    path = QString("stream://data.local") + path.right(path.length() - indexAttachmentsFolder + 1);

    return path.toUtf8();
}
