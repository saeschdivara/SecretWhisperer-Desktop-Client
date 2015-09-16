#ifndef PROTOCOLCONTROLLER_H
#define PROTOCOLCONTROLLER_H

#include <QObject>
#include <QtNetwork/QSslSocket>

#include "chat/connecteduser.h"
#include "chat/encryptor.h"

class ProtocolController : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolController(QObject *parent = 0);
    void prepareConnection(QSslSocket * socket);

    // Contact
    ConnectedUser * createUser();
    void createSymmetricKeyForUser(ConnectedUser * user);

    // Encryption
    QByteArray encryptWithAsymmetricKey(ConnectedUser * user, std::string & data);

signals:

public slots:
    void onServerDataEvent(QByteArray & data);

private:
    Encryptor * encryptor;
};

#endif // PROTOCOLCONTROLLER_H
