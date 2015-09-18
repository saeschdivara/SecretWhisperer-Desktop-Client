#include "migrationhelper.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include "chat/helper/sql.h"

MigrationHelper::MigrationHelper(QSqlDatabase database, QObject *parent) : QObject(parent),
    database(database)
{

}

bool MigrationHelper::hasMigrationsTable()
{
    QSqlQuery query("SELECT COUNT(*) FROM system_migration", database);
    return query.next();
}

void MigrationHelper::createMigrationsTable()
{
    loadTable(database, QString("migrations"));
}

void MigrationHelper::executeMigrations()
{
    QString baseMigrationsPath(":/sql/migrations/");
    QDir dir(baseMigrationsPath, "*.sql");

    for ( QString entry : dir.entryList() ) {
        qDebug() << entry;

        QString entryNumber = entry.left( entry.length() - 4 );

        bool isConversionSucceded;
        int orderNumber = entryNumber.toInt(&isConversionSucceded);

        if ( !isConversionSucceded ) {
            qWarning() << "The following file has no number: " << entry;
            continue;
        }

        QSqlQuery searchQuery(QString("SELECT * FROM system_migrations WHERE order_number = %1").arg(orderNumber), database);
        if ( searchQuery.next() ) {
            qDebug() << "Migration has already been done";
            continue;
        }

        QFile migrationFile(baseMigrationsPath + entry);

        if ( !migrationFile.open(QIODevice::ReadOnly) ) {
            qCritical() << "Could not open migration file: " << migrationFile.errorString();
            break;
        }

        QByteArray migrationData = migrationFile.readAll();
        migrationFile.close();

        QSqlQuery query(migrationData, database);

        if ( !query.exec() ) {
            qCritical() << "Migration query failed: " << query.lastError();
            break;
        }
    }
}

