#include <QApplication>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebInspector>

#include <botan/aes.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Botan::AES_256 aes;

    QWebView webview;
    webview.load(QUrl(QStringLiteral("qrc:/ui/interface/index.html")));
    webview.resize(640, 540);
    webview.show();

#if WHISPERER_DEBUG_BUILD
    webview.page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    QWebInspector inspector;
    inspector.setPage(webview.page());
    inspector.setVisible(false);
#endif

    return app.exec();
}
