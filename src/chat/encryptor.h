#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H

#include <QtCore/QObject>

#include <botan/botan.h>
#include <botan/pubkey.h>
#include <botan/pkcs8.h>
#include <botan/pk_keys.h>
#include <botan/rsa.h>

struct KeyPair {
    Botan::Public_Key * publicKey;
    Botan::Private_Key * privateKey;
};

class Encryptor : public QObject
{
    Q_OBJECT
public:
    explicit Encryptor(QObject *parent = 0);

    KeyPair createAsymmetricKeys();

signals:

public slots:
};

#endif // ENCRYPTOR_H
