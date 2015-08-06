TEMPLATE = app
CONFIG   -= app_bundle

QT += widgets webkitwidgets

SOURCES += main.cpp \
    chat/chatcontroller.cpp

RESOURCES += \
    ui.qrc

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    ui/interface/index.html

unix|win32: LIBS += -L/usr/local/lib/ -lbotan-1.11

INCLUDEPATH += /usr/local/include/botan-1.11/
DEPENDPATH += /usr/local/include/botan-1.11/

CONFIG += c++11

# DEFINES
debug {
    DEFINES += WHISPERER_DEBUG_BUILD
}

HEADERS += \
    chat/chatcontroller.h
