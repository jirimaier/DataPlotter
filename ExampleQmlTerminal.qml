import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

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
                onClicked: send("hello")
        }
        Text{
            id: name
        }
    }
}
