TEMPLATE = app
CONFIG   -= app_bundle

QT += widgets webkitwidgets

CONFIG += c++11

SOURCES += main.cpp \
    chat/chatcontroller.cpp \
    chat/connecteduser.cpp

RESOURCES += \
    ui.qrc

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    ui/interface/index.html

# Lib Botan
unix|win32: LIBS += -L/usr/local/lib/ -lbotan-1.11

INCLUDEPATH += /usr/local/include/botan-1.11/
DEPENDPATH += /usr/local/include/botan-1.11/

# Lib Snortify
unix|win32: LIBS += -L/usr/local/lib/ -lsnore-qt5

INCLUDEPATH += /usr/local/include/
DEPENDPATH += /usr/local/include/

# DEFINES
debug {
    DEFINES += WHISPERER_DEBUG_BUILD
}

HEADERS += \
    chat/chatcontroller.h \
    chat/connecteduser.h
