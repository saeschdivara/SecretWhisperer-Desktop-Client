#include "networkaccessmanager.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

class LocalReply : public QNetworkReply
{
public:
    LocalReply(QString path) : QNetworkReply(0) {
        file.setFileName(path);
        file.open(QIODevice::ReadOnly);

        open(ReadOnly | Unbuffered);
        qint64 fileSize = file.size();
        setHeader(QNetworkRequest::ContentLengthHeader, QVariant(fileSize));

        QMetaObject::invokeMethod(this, "metaDataChanged", Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, "downloadProgress", Qt::QueuedConnection,
            Q_ARG(qint64, fileSize), Q_ARG(qint64, fileSize));

        QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

    virtual ~LocalReply() {
        file.close();
    }

    virtual void close() {
        QNetworkReply::close();
        file.close();
    }

    virtual void abort() {
    }

    virtual qint64 size() const {
        return file.size();
    }

    virtual qint64 bytesAvailable() const {
        return file.size();
    }

    virtual bool isSequential() const {
        return true;
    }

protected:
    qint64 readData(char *data, qint64 maxlen) {
        return file.read(data, maxlen);
    }

private:
    QFile file;
};

NetworkAccessManager::NetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent)
{

}

QNetworkReply *NetworkAccessManager::createRequest(QNetworkAccessManager::Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QString localStreamProtocol("stream://data.local/");
    QString path = request.url().path();
    if ( path.startsWith(localStreamProtocol) ) {
        path = path.replace(localStreamProtocol, "");
        QDir currentDirectory = QDir::current();

        if ( currentDirectory.exists(path) ) {
            return new LocalReply(currentDirectory.absoluteFilePath(path));
        }
        else {
            return QNetworkAccessManager::createRequest(op, request, outgoingData);
        }
    }
    else {
        return QNetworkAccessManager::createRequest(op, request, outgoingData);
    }

}

