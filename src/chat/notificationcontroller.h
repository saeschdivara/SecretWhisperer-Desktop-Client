#ifndef NOTIFICATIONCONTROLLER_H
#define NOTIFICATIONCONTROLLER_H

#include <QObject>


#ifdef USE_SNORTIFY
    // SNORTIFY
    #include <libsnore/snore.h>
    #include <libsnore/notification/notification.h>
    #include <libsnore/log.h>
#endif

class NotificationController : public QObject
{
    Q_OBJECT
public:
    explicit NotificationController(QObject *parent = 0);

signals:

public slots:
    void showNotification(const QString & title, const QString & text);

private:

#ifdef USE_SNORTIFY
    // Notfications
    Snore::SnoreCore &core;
    Snore::Icon icon;
    Snore::Application snoreApplication;
    Snore::Alert alert;
#endif
};

#endif // NOTIFICATIONCONTROLLER_H
