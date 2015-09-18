#include "identitycontroller.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QList>

#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>

#include "chat/helper/sql.h"

// CLASS

IdentityController::IdentityController(QObject *parent) : QObject(parent)
{
    //
}

void IdentityController::createUserIdentity()
{
    createDatabaseConnection();

    migrations = new MigrationHelper(database, this);
    createDatabaseTables();
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

    // Create migrations
    if ( !migrations->hasMigrationsTable() ) {
        // Create table
        migrations->createMigrationsTable();
    }

    migrations->executeMigrations();
}
