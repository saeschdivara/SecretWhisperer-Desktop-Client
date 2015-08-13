#include <QApplication>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebInspector>

#include "chat/chatcontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ChatController chat;

    QWebView webview;
    webview.load(QUrl(QStringLiteral("qrc:/ui/interface/index.html")));
    webview.resize(640, 540);
    webview.show();

    // Expose objects to javascript
    webview.page()->mainFrame()->addToJavaScriptWindowObject(QStringLiteral("chat"), &chat);

#if WHISPERER_DEBUG_BUILD
    webview.page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    QWebInspector inspector;
    inspector.setPage(webview.page());
    inspector.setVisible(false);
#endif

    //Set application name to distinguish settings for each app
    app.setApplicationName("Secret-Whisperer");

    return app.exec();
}
