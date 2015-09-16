#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QtNetwork/QSslSocket>

class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(QSslSocket *socket, QObject *parent = 0);

    // Connection
    void listen();
    void send(const QByteArray & data);

signals:
    void newData(QByteArray &);
    void closed();

public slots:
    void onData();
    void onClose();

private:
    QSslSocket * socket;
};

#endif // CONNECTOR_H