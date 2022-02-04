import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    function send(data) {
        dataPlotter.transmitToSerial(data);
    }

    Connections {
        target: dataPlotter
        onReceivedFromSerial: {
            name.text = data
        }
    }

    Column {
        Rectangle {
            width:50
            height:30
            color:"red"
        }
        Text {
            text: dataPlotter.testText
        }
        Button {
            text: "Send hello"
            onClicked: {
                send("hello")
                popup.open();
            }
        }
        Text{
            id: name
        }
        Popup {
            id:popup
            x:0
            y:0
            width:100
            height:100
            Rectangle {
                width:50
                height:30
                color:"red"
            }
            Rectangle {
                width:50
                height:30
                color:"blue"
            }
        }
    }
}
