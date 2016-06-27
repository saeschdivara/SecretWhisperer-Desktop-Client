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
    ConnectedUser * createUserFromPublicKey(QByteArray & publicKey);
    void createSymmetricKeyForUser(ConnectedUser * user);

    // Encryption
    QByteArray encryptWithAsymmetricKey(ConnectedUser * user, std::string & data);
    QByteArray decryptWithAsymmetricKey(ConnectedUser * user, QByteArray & data);

    QByteArray encryptWithSymmetricKey(ConnectedUser * user, QByteArray & data);
    QByteArray decryptWithSymmetricKey(ConnectedUser * user, QByteArray & data);

signals:
    void signalAuthenticationSucceded();
    void signalStartup(const QByteArray & username, QByteArray & publicKey);
    void signalEncrypt(const QByteArray & username, QByteArray & message);
    void signalMessage(const QByteArray & username, QByteArray & message);
    void signalIdentityCheck(QByteArray &);
    void signalUserOnline(QString username);
    void signalError(QByteArray);

public slots:
    void onServerDataEvent(QByteArray & data);

protected:
    // Helper methods
    QByteArray prepareRequest(QByteArray & data, QByteArray command);
    QByteArray stripRequest(QByteArray data, QByteArray command);

private:
    Encryptor * encryptor;
};

#endif // PROTOCOLCONTROLLER_H
