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

signals:
    void connected();

public slots:
    void connectToServer(const QString &url, quint16 port);

private:
    QSslSocket * socket;
};

#endif // CHATCONTROLLER_H
