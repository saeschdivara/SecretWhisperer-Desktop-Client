#include "connector.h"

Connector::Connector(QSslSocket *socket, QObject *parent) : QObject(parent),
    socket(socket)
{
}

void Connector::listen()
{
    connect( socket, &QTcpSocket::readyRead, this, &Connector::onData );
    connect( socket, &QTcpSocket::disconnected, this, &Connector::onClose );
}

void Connector::send(const QByteArray &data)
{
    socket->write(data);
}

void Connector::onData()
{
    QByteArray data = socket->readAll();
    emit newData(data);
}

void Connector::onClose()
{
    emit closed();
}

