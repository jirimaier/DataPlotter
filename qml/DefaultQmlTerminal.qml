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

        Rectangle {
            id:terminal
            Material.theme: Material.Dark
            Layout.fillWidth: true
            Layout.fillHeight: visible
            color:"black"
            visible: ansiTerminalModel.active
            GridView {
                anchors.fill: parent
                id: terminalView
                model: ansiTerminalModel
                cellWidth: width*0.9999/model.columns
                cellHeight: cellWidth*1.3
                flow: GridView.LeftToRight
                clip: true
                delegate: Rectangle {
                    width: terminalView.cellWidth
                    height: terminalView.cellHeight
                    color: backgroundColor

                    Text {
                        anchors.centerIn: parent
                        text: character
                        color: fontColor
                        font.bold: bold
                        font.family: "Monospace"
                        font.pixelSize: parent.height*0.8
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        height: parent.height/20
                        color: fontColor
                        visible: underline
                    }

                    Rectangle {
                        id: buttonOverlap
                        visible: clickable
                        anchors.fill: parent
                        color: clickHighLightColor
                        opacity: buttonOverlap_ma.containsPress?0.5:buttonOverlap_ma.containsMouse?0.2:0
                        MouseArea {
                            id: buttonOverlap_ma
                            hoverEnabled: true
                            anchors.fill: parent
                            onClicked: send(character)
                        }
                    }
                }
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    width: parent.cellWidth
                    anchors.right: parent.right
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: !terminal.visible
            Layout.preferredHeight: parent.height/5
            color: "transparent"
            border.color: darkThemeIsUsed ? "gray" : "gray"
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
