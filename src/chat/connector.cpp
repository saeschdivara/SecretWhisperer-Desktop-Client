#include "connector.h"

const QByteArray endLiteral("\r\n\r\n");

Connector::Connector(QSslSocket *socket, QObject *parent) : QObject(parent),
    socket(socket)
{
}

void Connector::listen()
{
    connect( socket, &QTcpSocket::readyRead, this, &Connector::onData );
    connect( socket, &QTcpSocket::bytesWritten, this, &Connector::onDataWritten );
    connect( socket, &QTcpSocket::disconnected, this, &Connector::onClose );
}

void Connector::send(const QByteArray &data)
{
    socket->write(data);
}

void Connector::onMessage(const QByteArray &command, const QByteArray &data)
{
    send(
        command +
        data +
        endLiteral
    );
}

void Connector::onMessage(const QByteArray &command, const QByteArray &username, const QByteArray &data)
{
    const QByteArray seperator("\r\n");

    send(
        command +
        username +
        seperator +
        data +
        endLiteral
    );
}

void Connector::onData()
{
    QByteArray readData = socket->read(1024 * 4);
    savedData += readData;
    qDebug() << "New data gotten";
    qDebug() << "Available bytes: " << socket->bytesAvailable();
    qDebug() << "Bytes sizes: " << readData.size();

    if ( savedData.contains(endLiteral) ) {
        qDebug() << "End found";
        QByteArray data(savedData);
        data += "";
        savedData.clear();
        emit newData(data);
    }
}

void Connector::onClose()
{
    emit closed();
}

void Connector::onDataWritten(qint64 bytesNumber)
{
    qDebug() << "Bytes written: " << bytesNumber;
}

