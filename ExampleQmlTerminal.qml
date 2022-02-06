import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    anchors.fill: parent

    FontMetrics {
        id: fontMetrics
        font.pointSize: 10
        // Use fontMetrics.height to get pixel height of font with specified pointSize (for DPI scaling)
    }

    function send(data) {
        // Use this function to send data to serial port
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

    // Use $$V(name):(value) to set values of properties
    property int a: 0
    property int b: 0

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Rectangle{
            anchors.horizontalCenter: parent.horizontalCenter

            height: fontMetrics.height*2
            width: title.implicitWidth+20

            color: "yellow"

            Text {
                id: title
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: "Welcome to new terminal!"
            }
        }

        Text {
            text: 'This GUI can be created using QML file that can be sent from connected device using $$Q message type. Click <html><style type="text/css"></style><a href="undefinedlink">here</a></html> do open development options. You can save this as template for creating your terminal. Use the load and refresh button to test your file. Data in $$Q message are compressed and bytestuffed - use the export function to do that.'
            onLinkActivated: sendToParser("$$Sqmldev:1;")
            width: parent.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }

        RowLayout {
            Button{
                text: "Send this to serial port:"
                onClicked: send(txText.text)
            }
            TextInput {
                id: txText
                Layout.fillWidth: true
                text: "Hello World"
            }
        }

        RowLayout {
            Text{
                text: "This was received ($$D):"
            }
            TextInput {
                id: rxText
                Layout.fillWidth: true
                text: ""
            }
        }

        Text{
            // Display values of properties like this:
            text: "Variables: a=" + a + "  b=" + b +"  a×b=" + (a*b)
        }
    }
}
