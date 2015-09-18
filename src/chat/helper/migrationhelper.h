#ifndef MIGRATIONHELPER_H
#define MIGRATIONHELPER_H

#include <QObject>
#include <QtSql/QSqlDatabase>

class MigrationHelper : public QObject
{
    Q_OBJECT
public:
    explicit MigrationHelper(QSqlDatabase database, QObject *parent = 0);

    bool hasMigrationsTable();
    void createMigrationsTable();

    void executeMigrations();

signals:

public slots:

private:
    QSqlDatabase database;
};

#endif // MIGRATIONHELPER_H
