import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2

import QtWebEngine 1.0

ApplicationWindow {
    title: qsTr("Hello World")
    width: 640
    height: 480
    visible: true


    WebEngineView {
        id: webview
        url: "qrc:/ui/index.html"
        anchors.fill: parent
    }
}
