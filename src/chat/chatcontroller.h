#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include <QtNetwork/QSslSocket>

class ChatController : public QObject
{
    Q_OBJECT
public:
    explicit ChatController(QObject *parent = 0);
    virtual ~ChatController();

signals:
    void connected();
    void error(QAbstractSocket::SocketError);

public slots:

    void onConnectionEstablished();
    void onServerData();
    void onError(const QList<QSslError> &errors);

    // User Actions
    void connectToServer(const QString &url, quint16 port);
    void chooseUserName(const QString & username);
    void connectToUser(const QString & username);
    void sendMessageToUser(const QString & username, const QString & message);

protected:
    QByteArray stripRequest(QByteArray data, QByteArray command);

private:
    QSslSocket * socket;
};

#endif // CHATCONTROLLER_H
