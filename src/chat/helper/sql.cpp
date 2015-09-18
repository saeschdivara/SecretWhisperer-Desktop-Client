#include "chat/helper/sql.h"

#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

void loadTable(QSqlDatabase database, QString tableFileName) {
    QFile file(QString(":/sql/") + tableFileName + QString(".sql"));

    if ( !file.open(QIODevice::ReadOnly) ) {
        qWarning() << "File could not be opened: " << file.errorString();
    }

    QByteArray sqlData = file.readAll();
    file.close();

    QSqlQuery query(sqlData, database);

    if ( !query.exec() ) {
        qWarning() << "Executon failed: " << query.lastError();
    }
}
