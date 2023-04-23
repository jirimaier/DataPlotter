// These are maximum versions on Windows XP (Qt 5.7.1)
import QtQuick 2.7
import QtQml 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.2

import "qrc:/qml/DataPlotterComponents"

Rectangle {
    // Do not set size or anchors of root object

    property bool darkThemeIsUsed: dataPlotter.darkThemeIsUsed
    Material.theme: darkThemeIsUsed ? Material.Dark : Material.Light
    Material.accent: Material.Blue
    Material.elevation: 1
    color: dataPlotter.tabBackground

    function send(data) {
        // Use this function to send data to serial port
        if(data==="")
            return;
        dataPlotter.transmitToSerial(data);
    }

    function sendToParser(data) {
        // Use this function to send data into manual input parser
        dataPlotter.sendToParser(data);
    }

    Connections {
        target: dataPlotter
        function onReceivedFromSerial(data) {
            // This function handles "$$D.......\0" data received
            rxText.text = data
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        ANSITerminal {
            id: terminal
            Layout.fillWidth: true
            Layout.fillHeight: visible
            visible: ansiTerminalModel.active
        }

        MessageConsole {
            id: messageConsole
            Layout.fillWidth: true
            Layout.fillHeight: !terminal.visible
            Layout.preferredHeight: parent.height/5
        }

        SerialSender {
            Layout.fillWidth: true
            Layout.maximumHeight: parent.height/2
            Layout.preferredHeight: contentHeight
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 10
            onDataSent: messageConsole.autoscroll = true
        }
    }
}
