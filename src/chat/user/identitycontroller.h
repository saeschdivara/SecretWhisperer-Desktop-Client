#ifndef IDENTITYCONTROLLER_H
#define IDENTITYCONTROLLER_H

#include <QObject>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlTableModel>

#include "chat/encryptor.h"
#include "chat/helper/migrationhelper.h"

class IdentityController : public QObject
{
    Q_OBJECT
public:
    explicit IdentityController(QObject *parent = 0);

    // User specific
    void createUserIdentity(const QString &username, const QString &password);

    ConnectedUser * getUser() {
        return user;
    }

    std::string getSymmetricKeyString();

    // Contact specific
    void addContact(const QString & username, const QByteArray & publicKey);
    QHash<QString, ConnectedUser *> getContacts();

protected:
    void createDatabaseConnection();
    void createDatabaseTables();

private:
    // Database
    QSqlDatabase database;

    // Identity
    ConnectedUser * user;

    // Encryption
    Encryptor * encryptor;

    // Helpers
    MigrationHelper * migrations;
};

#endif // IDENTITYCONTROLLER_H
