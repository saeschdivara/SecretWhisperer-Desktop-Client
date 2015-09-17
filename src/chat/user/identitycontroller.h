#ifndef IDENTITYCONTROLLER_H
#define IDENTITYCONTROLLER_H

#include <QObject>

class IdentityController : public QObject
{
    Q_OBJECT
public:
    explicit IdentityController(QObject *parent = 0);

signals:

public slots:
};

#endif // IDENTITYCONTROLLER_H
