TEMPLATE = app

QT += qml quick widgets webenginewidgets

SOURCES += main.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
    ui/interface/index.html

unix|win32: LIBS += -L/usr/local/lib/ -lbotan-1.11

INCLUDEPATH += /usr/local/include/botan-1.11/
DEPENDPATH += /usr/local/include/botan-1.11/

CONFIG += c++11
