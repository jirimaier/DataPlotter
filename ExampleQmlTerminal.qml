import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    // Do not set size of anchors of root object

    property var themePalette: dataPlotter.themePalette // https://doc.qt.io/qt-5/qml-palette.html
    property bool darkThemeIsUsed: dataPlotter.darkThemeIsUsed
    property var textColor: themePalette.text

    color: dataPlotter.tabBackground

    property int fontPointSize: 9

    FontMetrics {
        id: fontMetrics
        font.pointSize: fontPointSize
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

    property int w: 0
    property int h: 0

    Column {
        anchors.fill: parent
        spacing: 10
        anchors.margins: 2

        Rectangle{
            anchors.horizontalCenter: parent.horizontalCenter

            height: fontMetrics.height*2
            width: title.implicitWidth+20

            color: darkThemeIsUsed?"yellow":"green"

            Text {
                id: title
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: "Welcome to new terminal!"
                font.pointSize: fontPointSize
            }
        }

        Text {
            text: 'This GUI can be created using QML file that can be sent from connected device using $$Q message type. Click <html><a href="null">here</a></html> do open development options. You can save this as template for creating your terminal. Use the load and refresh button to test your file. Data in $$Q message are compressed and bytestuffed - use the export function to do that.'
            onLinkActivated: sendToParser("$$Sqmldev:1;")
            width: parent.width
            wrapMode: Text.WordWrap
            font.pointSize: fontPointSize
            color: textColor
            linkColor: themePalette.link
            horizontalAlignment: Text.AlignJustify
        }

        RowLayout {
            Button{
                text: "Send this to serial port:"
                font.pointSize: fontPointSize
                onClicked: send(txText.text)
                palette: themePalette
            }
            TextInput {
                id: txText
                Layout.fillWidth: true
                text: "Hello World"
                font.pointSize: fontPointSize
                color: textColor
            }
        }

        RowLayout {
            Text{
                text: "This was received ($$D):"
                font.pointSize: fontPointSize
                color: textColor
            }
            TextInput {
                id: rxText
                Layout.fillWidth: true
                text: ""
                font.pointSize: fontPointSize
                color: textColor
            }
        }

        Text{
            // Display values of properties like this:
            text: "Variables: a=" + a + "  b=" + b +"  a×b=" + (a*b)
            font.pointSize: fontPointSize
            color: textColor
        }
    }
}
