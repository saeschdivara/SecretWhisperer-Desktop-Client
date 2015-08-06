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

    void onError(const QList<QSslError> &errors);

    void connectToServer(const QString &url, quint16 port);
    void connectToUser(const QString & username);

private:
    QSslSocket * socket;
};

#endif // CHATCONTROLLER_H
