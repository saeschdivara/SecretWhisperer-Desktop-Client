#include <QApplication>

#include <QtWebEngineWidgets/QWebEngineProfile>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebChannel/QWebChannel>

#include "chat/chatcontroller.h"
#include "chat/network/datanetworkhandler.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ChatController chat;

    QWebEngineView webview;
    QWebEnginePage * page = webview.page();

    page->profile()->installUrlSchemeHandler("stream", new DataNetworkHandler);

    webview.load(QUrl(QStringLiteral("qrc:/ui/interface/index.html")));
    webview.resize(640 * 2, 540);
    webview.show();

    // TODO: Data handling

    // Expose objects to javascript
    QWebChannel channel;
    page->setWebChannel(&channel);
    page->webChannel()->registerObject(QStringLiteral("chat"), &chat);

    // Set chat server url
    if ( app.arguments().size() >= 2 ) {
        QString url = app.arguments().at(1);
        QString command = QString("window.CHAT_SERVER_URL = '%1'").arg(url);
        page->runJavaScript(command);
    }

    //Set application name to distinguish settings for each app
    app.setApplicationName("Secret-Whisperer");

    return app.exec();
}
