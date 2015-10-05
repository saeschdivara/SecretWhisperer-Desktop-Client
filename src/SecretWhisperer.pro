TEMPLATE = app
CONFIG   -= app_bundle

QT += widgets webkitwidgets sql

CONFIG += c++11

SOURCES += main.cpp \
    chat/chatcontroller.cpp \
    chat/connecteduser.cpp \
    chat/connector.cpp \
    chat/encryptor.cpp \
    chat/protocolcontroller.cpp \
    chat/notificationcontroller.cpp \
    chat/user/identitycontroller.cpp \
    chat/helper/migrationhelper.cpp \
    chat/helper/sql.cpp \
    chat/network/networkaccessmanager.cpp

RESOURCES += \
    ui.qrc

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    ui/interface/index.html

# Lib Botan: https://github.com/randombit/botan
unix|win32: LIBS += -L/usr/local/lib/ -lbotan-1.11

INCLUDEPATH += /usr/local/include/botan-1.11/
DEPENDPATH += /usr/local/include/botan-1.11/

#DEFINES += USE_SNORTIFY

if (defined(USE_SNORTIFY)) {

    # Lib Snortify: https://github.com/Snorenotify/Snorenotify
    if(linux-g++*): {
       unix|win32: LIBS += -L/usr/local/lib/x86_64-linux-gnu/ -lsnore-qt5
    }
    else {
       unix|win32: LIBS += -L/usr/local/lib/ -lsnore-qt5
    }

    INCLUDEPATH += /usr/local/include/
    DEPENDPATH += /usr/local/include/

}

# DEFINES
debug {
    DEFINES += WHISPERER_DEBUG_BUILD
}

HEADERS += \
    chat/chatcontroller.h \
    chat/connecteduser.h \
    chat/connector.h \
    chat/encryptor.h \
    chat/protocolcontroller.h \
    chat/notificationcontroller.h \
    chat/user/identitycontroller.h \
    chat/helper/migrationhelper.h \
    chat/helper/sql.h \
    chat/network/networkaccessmanager.h
