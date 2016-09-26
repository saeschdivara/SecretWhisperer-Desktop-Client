#ifndef DATANETWORKHANDLER_H
#define DATANETWORKHANDLER_H

#include <QtWebEngineCore/QWebEngineUrlSchemeHandler>


class DataNetworkHandler : public QWebEngineUrlSchemeHandler
{
    Q_OBJECT
public:
    DataNetworkHandler(QObject *parent = 0);


    virtual void requestStarted(QWebEngineUrlRequestJob *request);
};

#endif // DATANETWORKHANDLER_H
