#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <QObject>
#include <QtNetwork/QTcpSocket>

class Connector : public QObject
{
    Q_OBJECT
public:
    explicit Connector(QTcpSocket * socket, QObject *parent = 0);
    void listen();

    void send(const QByteArray & data);

signals:
    void newData(QByteArray &);
    void closed();

public slots:
    void onData();
    void onClose();

private:
    QTcpSocket * socket;
};

#endif // CONNECTOR_H
