#include <QApplication>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKitWidgets/QWebInspector>

#include "chat/chatcontroller.h"
#include "chat/network/networkaccessmanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ChatController chat;

    QWebView webview;
    QWebPage * page = webview.page();

    NetworkAccessManager manager;
    page->setNetworkAccessManager(&manager);

    webview.load(QUrl(QStringLiteral("qrc:/ui/interface/index.html")));
    webview.resize(640 * 2, 540);
    webview.show();

    page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    QObject::connect( page, &QWebPage::linkClicked, &chat, &ChatController::onLinkClicked );

    // Expose objects to javascript
    page->mainFrame()->addToJavaScriptWindowObject(QStringLiteral("chat"), &chat);

    // Set chat server url
    if ( app.arguments().size() >= 2 ) {
        QString url = app.arguments().at(1);
        QString command = QString("window.CHAT_SERVER_URL = '%1'").arg(url);
        webview.page()->mainFrame()->evaluateJavaScript(command);
    }

#if WHISPERER_DEBUG_BUILD
    page->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    QWebInspector inspector;
    inspector.setPage(webview.page());
    inspector.setVisible(false);
#endif

    //Set application name to distinguish settings for each app
    app.setApplicationName("Secret-Whisperer");

    return app.exec();
}
