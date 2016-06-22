#ifndef CONNECTEDUSER_H
#define CONNECTEDUSER_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#include <botan/symkey.h>

namespace Botan {
    class Public_Key;
    class Private_Key;
}

class ConnectedUser : public QObject
{
    Q_OBJECT
public:
    explicit ConnectedUser(Botan::Public_Key *publicKey,
                           Botan::Private_Key *privateRsaKey = Q_NULLPTR,
                           bool hasUserInitieted = true);

    Botan::Public_Key & publicKey() {
        return *publicRsaKey;
    }

    Botan::Private_Key & privateKey() {
        return *privateRsaKey;
    }

    void setSymmetricKey(Botan::SymmetricKey key) {
        encryptionKey = key;
    }

    Botan::SymmetricKey & symmetricKey() {
        return encryptionKey;
    }


    QByteArray getDataFromPublicKey();

    QString userName() const {
        return _username;
    }

    void setUserName(const QString & userName) {
        _username = userName;
    }

signals:

public slots:

private:
    Botan::Public_Key *publicRsaKey;
    Botan::Private_Key *privateRsaKey;

    bool hasUserInitieted;

    Botan::SymmetricKey encryptionKey;

    QString _username;
};

#endif // CONNECTEDUSER_H
