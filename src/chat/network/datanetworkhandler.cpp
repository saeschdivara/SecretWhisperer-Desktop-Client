#include "datanetworkhandler.h"

#include <QtCore/QBuffer>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QMimeDatabase>
#include <QtCore/QMimeType>
#include <QtWebEngineCore/QWebEngineUrlRequestJob>

DataNetworkHandler::DataNetworkHandler(QObject * parent) : QWebEngineUrlSchemeHandler(parent)
{

}

void DataNetworkHandler::requestStarted(QWebEngineUrlRequestJob *request)
{
    QUrl requestUrl = request->requestUrl();
    QString url = requestUrl.url().replace("stream://data.local/", "");
    qDebug() << url;

    QDir currentDirectory = QDir::current();
    QString fileName =  currentDirectory.path() + QDir::separator() + url;
    QFile * requestedFile = new QFile(fileName);

    if ( requestedFile->exists() ) {

        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(fileName);

        QByteArray fileMime = mime.name().toUtf8();
        //request->reply(fileMime, requestedFile);

        requestedFile->open(QIODevice::ReadOnly);

        QBuffer *buffer = new QBuffer;
        connect(request, SIGNAL(destroyed()), buffer, SLOT(deleteLater()));

        buffer->open(QIODevice::WriteOnly);
        buffer->write(requestedFile->readAll());
        buffer->close();

        requestedFile->close();

        request->reply(fileMime, buffer);
    }
    else {
        request->fail(QWebEngineUrlRequestJob::Error::UrlNotFound);
    }
}
