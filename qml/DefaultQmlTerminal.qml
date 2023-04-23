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
            Layout.fillWidth: true
            Layout.fillHeight: !terminal.visible
            Layout.preferredHeight: parent.height/5
        }

        Flickable {
            id: flickable
            Layout.fillWidth: true
            Layout.preferredHeight: columnLayout.implicitHeight
            Layout.maximumHeight: parent.height/2
            Layout.leftMargin: 5
            Layout.rightMargin: 5
            Layout.topMargin: 10
            contentWidth: columnLayout.width
            contentHeight: columnLayout.height
            flickableDirection: Flickable.VerticalFlick
            clip: true

            ScrollIndicator.vertical: ScrollIndicator {}

            ColumnLayout {
                id: columnLayout
                width: flickable.width
                spacing: 0
                Repeater {
                    id: sendlines
                    property int count: 1
                    property int selectedEnding: 0
                    property var endings: ["\n","\r","\r\n",""]
                    property var endingTexts: ["\\n","\\r","\\r\\n"," "]
                    property string ending: endings[selectedEnding]

                    function addline() {
                        count++
                        linesModel.insert(0,{})
                    }

                    function removeline() {
                        linesModel.remove(0,1)
                        count--
                    }

                    model: ListModel {
                        id:linesModel
                        ListElement{}
                    }
                    RowLayout {
                        TextField {
                            id: inputTextField
                            function sendText() {
                                var data = text + sendlines.ending;
                                if(data === "")
                                    return;
                                send(data)
                                messageModel.addMessage(data.replace(/\n/g, "\\n").replace(/\r/g, "\\r"),'s');
                                messageList.autoScroll = true
                                if(index === sendlines.count-1)
                                    text = "";
                            }

                            onEditingFinished: if(focus) sendText()

                            Layout.fillWidth: true
                            placeholderText: "Send to device"
                        }

                        Button {
                            visible: index === sendlines.count-1
                            text: sendlines.endingTexts[sendlines.selectedEnding]
                            onClicked: sendlines.selectedEnding = (sendlines.selectedEnding+1)%4
                            font.capitalization: Font.MixedCase
                            //flat:true
                            implicitWidth: height
                        }

                        Button {
                            visible: index !== sendlines.count-1
                            text: "\u27A4"
                            onClicked: inputTextField.sendText()
                            implicitWidth: height
                        }

                        Button {
                            visible: index >= sendlines.count-2
                            text: index===sendlines.count-1 ? "+":"-"
                            onClicked: text == "+" ? sendlines.addline() : sendlines.removeline()
                            implicitWidth: height
                        }
                    }
                }
            }
        }
    }
}
