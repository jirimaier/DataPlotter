import QtQuick 2.7
import QtQml 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.2

ListView {
    property int linecount: model.count
    property int selectedEnding: 0
    property var endings: ["\n","\r","\r\n",""]
    property var endingTexts: ["\\n","\\r","\\r\\n"," "]
    property string ending: endings[selectedEnding]
    property string endingText: endingTexts[selectedEnding]

    signal dataSent

    clip: true

    model: ListModel {
        id:linesModel
        ListElement{dummy:""}
    }
    delegate: RowLayout {
        width: parent ? parent.width : 0
        property bool isLast: index === count-1
        Row {
            Layout.fillWidth: true
            TextField {
                id: inputTextField
                function sendText() {
                    var data = text + ending;
                    if(data === "")
                        return;
                    dataPlotter.transmitToSerial(data)
                    messageModel.addMessage(data.replace(/\n/g, "\\n").replace(/\r/g, "\\r"),'s');
                    if(isLast)
                        text = "";

                }

                onEditingFinished: if(focus) sendText()
                width: parent.width - endingfield.width
                placeholderText: "Send to device"
            }
            TextField {
                id: endingfield
                text: endingText
                readOnly: true
                width: fontMetrics.implicitWidth
                Text {
                    id:fontMetrics
                    visible: false
                    text:"\\r\\n"
                    font: parent.font
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: selectedEnding = (selectedEnding+1)%4
                }
            }
        }

        Button {
            visible: true
            text: "\u27A4"
            onClicked: inputTextField.sendText()
            implicitWidth: height
        }

        Button {
            visible: isLast
            text: "+"
            onClicked: {
                linesModel.append({dummy:""})
                positionViewAtEnd()
            }
            implicitWidth: height
        }

        Button {
            visible: !isLast
            text: "×"
            onClicked: linesModel.remove(index)
            implicitWidth: height
        }
    }
}
