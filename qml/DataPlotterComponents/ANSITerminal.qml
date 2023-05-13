import QtQuick 2.7
import QtQml 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.2

Rectangle {
    Material.theme: Material.Dark
    color:"black"
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
            border.width: (ansiTerminalModel.showGrid) ? (index===ansiTerminalModel.cursorIndex?3:1):0
            border.color: "gray"

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
                visible: clickable || (ansiTerminalModel.showGrid)
                anchors.fill: parent
                color: clickHighLightColor
                opacity: buttonOverlap_ma.containsPress?0.5:buttonOverlap_ma.containsMouse?0.2:0
                MouseArea {
                    id: buttonOverlap_ma
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: {
                        if(ansiTerminalModel.showGrid)
                            ansiTerminalModel.gridClicked(index)
                        dataPlotter.transmitToSerial(character)
                    }
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
