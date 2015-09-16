#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>
#include <QtCore/QHash>

#include <QtNetwork/QSslSocket>

// SNORTIFY
#include <libsnore/snore.h>
#include <libsnore/notification/notification.h>
#include <libsnore/log.h>

// LOCAL
#include "chat/connecteduser.h"
#include "chat/connector.h"
#include "chat/protocolcontroller.h"


class ChatController : public QObject
{
    Q_OBJECT
public:
    explicit ChatController(QObject *parent = 0);
    virtual ~ChatController();

signals:
    void connected();
    void receivedUserMessage(const QString & username, const QString & message);
    void connectionToUserEstablished(const QString & username);
    void error(QAbstractSocket::SocketError);

public slots:

    void onConnectionEstablished();
    void onServerData(QByteArray &data);
    void onError(const QList<QSslError> &errors);
    void onSocketError(QAbstractSocket::SocketError error);

    // User Actions
    void connectToServer(const QString &url, quint16 port);
    void chooseUserName(const QString & username);
    void connectToUser(const QString & username);
    void sendMessageToUser(const QString & username, const QString & message);

protected:

    // Events helper
    void onStartupEvent(const QByteArray & data);
    void onEncryptEvent(const QByteArray & data);
    void onMessageEvent(const QByteArray & data);

    // Helper methods
    QByteArray stripRequest(QByteArray data, QByteArray command);

private:
    // Server connection
    QSslSocket * socket;
    Connector * connector;
    ProtocolController * protocol;

    // Contacts
    QHash<QByteArray, ConnectedUser *> connectedUsers;

    // Notfications
    Snore::SnoreCore &core;
    Snore::Icon icon;
    Snore::Application snoreApplication;
    Snore::Alert alert;
};

#endif // CHATCONTROLLER_H
