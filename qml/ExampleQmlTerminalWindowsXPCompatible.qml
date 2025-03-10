import QtQuick 2.7
import QtQml 2.0
import QtQuick.Controls 1.4  // Use older Controls version for Qt 5.7.1
import QtQuick.Layouts 1.2

import "qrc:/qml/DataPlotterComponents"

Rectangle {
    property bool darkThemeIsUsed: dataPlotter.darkThemeIsUsed
    color: darkThemeIsUsed ? "#303030" : "#FFFFFF" // Manual theme implementation

    function send(data, format) {
        if (format === undefined) {
            format = "string";
        }
        dataPlotter.transmitToSerial(data, format);
    }

    function sendToParser(data) {
        dataPlotter.sendToParser(data);
    }

    Connections {
        target: dataPlotter
        onReceivedFromSerial: {
            rxText.text = data
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: mainColumn.implicitHeight
        Column {
            id: mainColumn
            anchors.fill: parent
            spacing: 10
            anchors.margins: 5

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                height: title.implicitHeight * 2
                width: title.implicitWidth + 20
                color: darkThemeIsUsed ? "green" : "yellow"
                Label {
                    id: title
                    anchors.centerIn: parent
                    text: "Welcome to QML terminal!"
                }
            }

            Label {
                text: "You can create controls that will send text commands to the device."
                wrapMode: Text.WordWrap
            }

            Row {
                Button {
                    text: "Send:"
                    onClicked: send(txText.text)
                }
                TextField {
                    id: txText
                    text: "Hello World"
                }
            }

            Label {
                text: "You can create controls that will send binary data to the device."
                wrapMode: Text.WordWrap
            }

            Button {
                text: "Send (as uint16):"
                onClicked: send(txNumber.value, "u16")
            }

            SpinBox {
                id: txNumber
                from: 0
                to: 65535
                value: 258
            }

            Label {
                text: "You can use $$V(property):(value) to set variables inside the QML file."
                wrapMode: Text.WordWrap
            }

            Label {
                text: "Variables: a=" + a + "  b=" + b + "  a×b=" + (a*b)
            }

            Label {
                text: "You can also create your own parser for messages sent as $$D(message)\\0"
                wrapMode: Text.WordWrap
            }

            Row {
                Label { text: "Received:" }
                TextField {
                    id: rxText
                    text: ""
                    readOnly: true
                }
            }

            Button {
                text: "Need more space?"
                onClicked: overlay.visible = true
            }
        }
    }

    Rectangle {
        id: overlay
        visible: false
        width: parent.width
        height: parent.height
        color: "#80000000" // Semi-transparent background
        MouseArea {
            anchors.fill: parent
            onClicked: overlay.visible = false
        }
        Column {
            anchors.centerIn: parent
            Label { text: "You can use a pop-up" }
            Slider { id: slider_r; from: 0; to: 255 }
            Slider { id: slider_g; from: 0; to: 255 }
            Slider { id: slider_b; from: 0; to: 255 }
            Rectangle {
                width: 100; height: 50
                color: Qt.rgba(slider_r.value/255., slider_g.value/255., slider_b.value/255., 1)
            }
            Button {
                text: "OK"
                onClicked: overlay.visible = false
            }
        }
    }

    function randomInteger(min, max) {
        return Math.floor(Math.random() * (max - min + 1)) + min;
    }
}
