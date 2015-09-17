#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>
#include <QtCore/QHash>

#include <QtNetwork/QSslSocket>

// LOCAL
#include "chat/connecteduser.h"
#include "chat/connector.h"
#include "chat/notificationcontroller.h"
#include "chat/protocolcontroller.h"


class ChatController : public QObject
{
    Q_OBJECT
public:
    explicit ChatController(QObject *parent = 0);
    virtual ~ChatController();

    void listenOnErrors();
    void listenOnProtocol();

signals:
    void connected();
    void receivedUserMessage(const QString & username, const QString & message);
    void connectionToUserEstablished(const QString & username);
    void error(QAbstractSocket::SocketError);

public slots:

    void onConnectionEstablished();
    void onError(const QList<QSslError> &errors);
    void onSocketError(QAbstractSocket::SocketError error);

    // User Actions
    void connectToServer(const QString &url, quint16 port);
    void chooseUserName(const QString & username);
    void connectToUser(const QString & username);
    void sendMessageToUser(const QString & username, const QString & message);

protected slots:

    // Events helper
    void onStartupEvent(const QByteArray & username, QByteArray &publicKey);
    void onEncryptEvent(const QByteArray & username, QByteArray &message);
    void onMessageEvent(const QByteArray & username, QByteArray &message);

private:
    // Server connection
    QSslSocket * socket;
    Connector * connector;
    NotificationController * notifyer;
    ProtocolController * protocol;

    // Contacts
    QHash<QByteArray, ConnectedUser *> connectedUsers;
};

#endif // CHATCONTROLLER_H
