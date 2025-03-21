import QtQuick 2.7
import QtQml 2.0
import QtQuick.Controls 2.0
import QtQuick.Controls.Material 2.0
import QtQuick.Layouts 1.2

Rectangle {
    color: "transparent"
    border.color: darkThemeIsUsed ? "gray" : "gray"
    border.width: 1
    property bool empty: messageModel.empty
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

            Rectangle {
                color: type === 'w' ? "red" : type === 'i' ? "darkgreen" : "blue"
                anchors.fill: ts
            }

            Text {
                id:ts
                text: time
                color: "white"
                anchors.left: parent.left
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                height: msg.implicitHeight
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

        ScrollIndicator.vertical: ScrollIndicator {
            onPositionChanged: {
                if(Qt.version >= "5.12.0") {
                    if(visualPosition+visualSize >= 1)
                        messageList.autoScroll = true;
                } else {
                    if(position >= 1-size)
                        messageList.autoScroll = true;
                }
            }
        }
    }
}
