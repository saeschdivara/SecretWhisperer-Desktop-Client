#include "notificationcontroller.h"

// QT
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>

#ifdef USE_SNORTIFY
    // SNORTIFY
    #include <libsnore/snore.h>
    #include <libsnore/notification/notification.h>
    #include <libsnore/log.h>
#endif

NotificationController::NotificationController(QObject *parent) : QObject(parent)

#ifdef USE_SNORTIFY
  // Snortify
  ,core(Snore::SnoreCore::instance()),
  icon(QString(":/root/snore.png")),
  snoreApplication(Snore::Application(qApp->applicationName(), icon)),
  alert(Snore::Alert(QString("Default"), icon))
#endif
{

#ifdef USE_SNORTIFY
    // Snortify
    Snore::SnoreLog::setDebugLvl(1);

    //Get the core
    Snore::SnoreCore::instance().loadPlugins(
                Snore::SnorePlugin::BACKEND | Snore::SnorePlugin::SECONDARY_BACKEND
    );

    //All notifications have to have icon, so prebuild one
    core.registerApplication(snoreApplication);

    //Also alert is mandatory, just choose the default one
    snoreApplication.addAlert(alert);
#endif
}

void NotificationController::showNotification(const QString &title, const QString &text)
{

#ifdef USE_SNORTIFY
    // Inform the user of the new message
    Snore::Notification n(snoreApplication, alert,
                   title,
                   text,
                   icon);

    // Optional: you can also set delivery date if you want to schedule notification
    //n.setDeliveryDate(QDateTime::currentDateTime().addSecs(5));

    core.broadcastNotification(n);
#else
    qDebug() << title << " => " << text;
#endif
}

