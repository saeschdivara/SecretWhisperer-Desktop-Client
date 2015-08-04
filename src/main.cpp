#include <QApplication>
#include <QQmlApplicationEngine>

#include <botan/aes.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Botan::AES_256 aes;

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
