#include "identitycontroller.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QList>

#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include "chat/helper/sql.h"

// CLASS

IdentityController::IdentityController(QObject *parent) : QObject(parent),
    encryptor(new Encryptor(this))
{
    //
}

void IdentityController::createUserIdentity(const QString & username, const QString & password)
{
    createDatabaseConnection();

    migrations = new MigrationHelper(database, this);
    createDatabaseTables();

    QSqlQuery query("SELECT * FROM user WHERE user_name = '" + username + "'", database);

    // Check if the users identity has not been generated yet
    if ( !query.next() ) {

        // Create public and private keys
        KeyPair pair = encryptor->createAsymmetricKeys();

        // Init user
        user = new ConnectedUser( pair.publicKey, pair.privateKey );
        user->setSymmetricKey( encryptor->createSymmetricKey() );
        user->setUserName(username);

        // Store user data in database
        std::string symmetricKey = user->symmetricKey().as_string();

        std::string pub = Botan::X509::PEM_encode(user->publicKey());
        std::string priv = Botan::PKCS8::PEM_encode(user->privateKey());

        QByteArray encryptedPublicKey = encryptor->encryptSymmetricly(
            symmetricKey,
            QByteArray::fromStdString(pub)
        );

        QByteArray encryptedPrivateKey = encryptor->encryptSymmetricly(
            symmetricKey,
            QByteArray::fromStdString(priv)
        );

        QByteArray encryptedSymmetricKey = encryptor->encryptSymmetricly(
            password.toStdString(),
            QByteArray::fromStdString(symmetricKey)
        );

        query.prepare("INSERT INTO user (user_name, public_key, private_key, symmetric_key) VALUES (?, ?, ?, ?)");
        query.addBindValue(username);
        query.addBindValue(encryptedPublicKey);
        query.addBindValue(encryptedPrivateKey);
        query.addBindValue(encryptedSymmetricKey);
        query.exec();
    }
    // Retrieve and decrypt identity of the user
    else {
        QSqlRecord record = query.record();

        // Decrypt keys
        QByteArray decryptedSymmetricKey;
        QByteArray decryptedPublicKey;
        QByteArray decryptedPrivateKey;

        // Symmetric key
        try {
            QByteArray encryptedSymmetricKey = record.value("symmetric_key").toByteArray();
            decryptedSymmetricKey = encryptor->decryptSymmetricly(password.toStdString(), encryptedSymmetricKey);
        }
        catch (std::exception error) {
            qCritical() << "Could not decrypt symmetric key: " << error.what();
            return;
        }

        // Public key
        try {
            QByteArray encryptedPublicKey = record.value("public_key").toByteArray();
            decryptedPublicKey = encryptor->decryptSymmetricly(decryptedSymmetricKey.toStdString(), encryptedPublicKey);
        }
        catch (std::exception error) {
            qCritical() << "Could not decrypt public key: " << error.what();
            return;
        }

        // Private key
        QByteArray encryptedPrivateKey = record.value("private_key").toByteArray();
        decryptedPrivateKey = encryptor->decryptSymmetricly(decryptedSymmetricKey.toStdString(), encryptedPrivateKey);

        // Key setup after decryption
        Botan::SymmetricKey symmetricKey(decryptedSymmetricKey.toStdString());

        Botan::DataSource_Memory key_pub(decryptedPublicKey.toStdString());
        Botan::DataSource_Memory key_priv(decryptedPrivateKey.toStdString());

        // Load keys
        Botan::AutoSeeded_RNG rng;
        auto publicRsaKey = Botan::X509::load_key(key_pub);
        auto privateRsaKey = Botan::PKCS8::load_key(key_priv, rng);

        user = new ConnectedUser( publicRsaKey, privateRsaKey );
        user->setSymmetricKey(symmetricKey);
        user->setUserName(username);
    }

}

std::string IdentityController::getSymmetricKeyString()
{
    return Botan::X509::PEM_encode(user->publicKey());
}

/**
 * @brief Adds contact data to the database
 *
 * @param username Contact username
 * @param publicKey Contact public key
 */
void IdentityController::addContact(const QString &username, const QByteArray &publicKey)
{
    QSqlQuery query(database);

    query.prepare("INSERT INTO user (user_name, public_key, connected_user) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(publicKey);
    query.addBindValue(user->userName());
    query.exec();
}

/**
 * @brief Retrieves all contacts for the current user from the database
 *
 * @return Returns contacts map where the key is the username
 */
QHash<QString, ConnectedUser *> IdentityController::getContacts()
{
    QHash<QString, ConnectedUser *> contacts;

    // Retrieve contacts from the database
    QSqlQuery query("SELECT * FROM contact WHERE connected_user = '" + user->userName() + "'", database);

    while(query.next()) {
        // Get database values
        QSqlRecord record = query.record();
        QString userName = record.value("user_name").toString();
        QByteArray publicKeyData = record.value("public_key").toByteArray();

        // Load public key
        Botan::DataSource_Memory key_pub(publicKeyData.toStdString());
        auto publicKey = Botan::X509::load_key(key_pub);

        ConnectedUser * contact = new ConnectedUser( publicKey );
        contact->setUserName(userName);

        contacts.insert(userName, contact);
    }

    return contacts;
}

void IdentityController::createDatabaseConnection()
{
    if ( !QSqlDatabase::contains(QStringLiteral("CHAT-DB")) ) {
        database = QSqlDatabase::addDatabase("QSQLITE", QStringLiteral("CHAT-DB"));
        database.setDatabaseName("data.db");
        database.open();
    }
    else {
        database = QSqlDatabase::database(QStringLiteral("CHAT-DB"));
    }
}

void IdentityController::createDatabaseTables()
{
    loadTable(database, QString("user"));
    loadTable(database, QString("contact"));

    // Create migrations
    if ( !migrations->hasMigrationsTable() ) {
        // Create table
        migrations->createMigrationsTable();
    }

    migrations->executeMigrations();
}
