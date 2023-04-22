// These are maximum versions on Windows XP (Qt 5.7.1)
import QtQuick 2.7
import QtQml 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.2

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
        messageModel.addMessage(data.replace(/\n/g, "\\n").replace(/\r/g, "\\r"),'s');
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
        spacing: 0
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: darkThemeIsUsed ? "white" : "black"
            border.width: 1
            ListView {
                anchors.fill: parent
                anchors.margins: 1
                property bool autoScroll: true
                clip: true
                id: messageList
                model: messageModel
                boundsBehavior: Flickable.StopAtBounds
                delegate: Rectangle {
                    width: messageList.width
                    height: msg.implicitHeight
                    color: index % 2 === 0 ? "transparent" : (darkThemeIsUsed?"darkgray":"lightgray")  // Set alternating row colors

                    Label {
                        id:ts
                        text: time
                        color: "white"
                        background: Rectangle {
                            color: type === 'w' ? "red" : type === 'i' ? "darkgreen" : "blue"
                        }
                        anchors.left: parent.left
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        height: parent.height
                        width: implicitWidth * 1.2
                        font.pointSize: 6
                    }

                    Text {
                        id:msg
                        text: message
                        color: "black"
                        anchors.left: ts.right
                        anchors.right: parent.right
                        anchors.rightMargin: 2
                        verticalAlignment: Text.AlignVCenter
                        padding: 5
                        wrapMode: Text.WordWrap
                    }
                }

                onContentHeightChanged: {
                    if(contentHeight>height) {
                        if(autoScroll)
                            positionViewAtEnd()
                    } else
                        autoScroll = true;
                }

                flickableDirection: Flickable.VerticalFlick

                // Disable automatic scrolling when the user scrolls manually
                onFlickStarted: {
                    autoScroll = false;
                }

                ScrollIndicator.vertical: ScrollIndicator {onPositionChanged: if(visualPosition+visualSize == 1) messageList.autoScroll = true }
            }
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
                Repeater {
                    model: linecount.currentValue
                    RowLayout {
                        TextField {
                            id: inputTextField
                            function sendText() {
                                send(text + lineending.currentValue)
                                if(linecount.currentValue===1)
                                    text = "";
                            }

                            onEditingFinished: if(focus) sendText()

                            Layout.fillWidth: true
                            placeholderText: "Send to device"
                        }
                        Button {
                            text: "Send"
                            onClicked: inputTextField.sendText()
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.margins: 5
            Layout.alignment: Qt.AlignRight
            ComboBox {
                id: lineending
                textRole: "text"
                valueRole: "value"
                model: ListModel {
                    ListElement {value: "\n"; text: "LF"}
                    ListElement {value: "\r\n"; text: "CRLF"}
                    ListElement {value: "\r"; text: "CR"}
                    ListElement {value: ""; text: "None"}
                }
            }

            ComboBox {
                id: linecount
                textRole: "text"
                valueRole: "value"
                model: ListModel {
                    ListElement {value: 1; text: "Single"}
                    ListElement {value: 3; text: "Multiple"}
                    ListElement {value: 8; text: "Many"}
                }
            }
        }
    }
}
