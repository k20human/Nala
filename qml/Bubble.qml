import QtQuick
import QtQuick.Window

Window {
    id: bubbleRoot

    objectName: "bubbleWindow"
    title: "Nala Bubble"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"
    visible: false

    width: 250
    height: 64

    property string message: ""
    property bool arrowAtTop: false
    property bool arrowAtRight: false
    property real arrowX: width * 0.5
    property real arrowY: height * 0.5

    function appear() {
        bubbleCard.opacity = 1.0;
        bubbleCard.scale = 1.0;
        arrowCanvas.requestPaint();
        dismissTimer.restart();
    }

    function dismiss() {
        bubbleCard.opacity = 0.0;
        bubbleCard.scale = 0.88;
        closeTimer.restart();
    }

    Item {
        id: bubbleCard
        anchors.fill: parent
        anchors.margins: 4
        opacity: 0.0
        scale: 0.88

        Behavior on opacity {
            NumberAnimation {
                duration: 220
                easing.type: Easing.OutCubic
            }
        }
        Behavior on scale {
            NumberAnimation {
                duration: 260
                easing.type: Easing.OutBack
            }
        }

        // Main card body
        Rectangle {
            id: cardBody
            anchors.fill: parent
            anchors.topMargin: (!bubbleRoot.arrowAtRight && bubbleRoot.arrowAtTop) ? 8 : 0
            anchors.bottomMargin: (!bubbleRoot.arrowAtRight && !bubbleRoot.arrowAtTop) ? 8 : 0
            anchors.rightMargin: bubbleRoot.arrowAtRight ? 8 : 0
            radius: 14
            color: "#181825"
            border.color: backend.mascotColor
            border.width: 1.5
            antialiasing: true
            z: 1

            Text {
                id: msgLabel
                anchors.fill: parent
                anchors.margins: 8
                text: bubbleRoot.message
                color: "#cdd6f4"
                font.pixelSize: 13
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
                z: 2
            }
        }

        // Pointer triangle seamlessly fused with the card border
        Canvas {
            id: arrowCanvas
            z: 2
            antialiasing: true

            property bool atRight: bubbleRoot.arrowAtRight
            property bool atTop: bubbleRoot.arrowAtTop
            property real ax: bubbleRoot.arrowX
            property real ay: bubbleRoot.arrowY
            property color arrowColor: backend.mascotColor

            width: atRight ? 12 : 20
            height: atRight ? 20 : 12

            x: atRight ? (cardBody.x + cardBody.width - 2) : (ax - 4 - 10)
            y: atRight ? (ay - 4 - 10) : (atTop ? (cardBody.y - 10) : (cardBody.y + cardBody.height - 2))

            onAtRightChanged: requestPaint()
            onAtTopChanged: requestPaint()
            onAxChanged: requestPaint()
            onAyChanged: requestPaint()
            onArrowColorChanged: requestPaint()

            onPaint: {
                var ctx = getContext("2d");
                ctx.reset();
                ctx.clearRect(0, 0, width, height);

                ctx.fillStyle = "#181825";
                ctx.strokeStyle = arrowColor;
                ctx.lineWidth = 1.5;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";

                if (atRight) {
                    ctx.beginPath();
                    ctx.moveTo(1.5, 1);
                    ctx.lineTo(10.5, 10);
                    ctx.lineTo(1.5, 19);
                    ctx.lineTo(0, 19);
                    ctx.lineTo(0, 1);
                    ctx.closePath();
                    ctx.fill();

                    ctx.beginPath();
                    ctx.moveTo(1.5, 1);
                    ctx.lineTo(10.5, 10);
                    ctx.lineTo(1.5, 19);
                    ctx.stroke();
                } else if (atTop) {
                    ctx.beginPath();
                    ctx.moveTo(1, 10.5);
                    ctx.lineTo(10, 1.5);
                    ctx.lineTo(19, 10.5);
                    ctx.lineTo(19, 12);
                    ctx.lineTo(1, 12);
                    ctx.closePath();
                    ctx.fill();

                    ctx.beginPath();
                    ctx.moveTo(1, 10.5);
                    ctx.lineTo(10, 1.5);
                    ctx.lineTo(19, 10.5);
                    ctx.stroke();
                } else {
                    ctx.beginPath();
                    ctx.moveTo(1, 1.5);
                    ctx.lineTo(10, 10.5);
                    ctx.lineTo(19, 1.5);
                    ctx.lineTo(19, 0);
                    ctx.lineTo(1, 0);
                    ctx.closePath();
                    ctx.fill();

                    ctx.beginPath();
                    ctx.moveTo(1, 1.5);
                    ctx.lineTo(10, 10.5);
                    ctx.lineTo(19, 1.5);
                    ctx.stroke();
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: bubbleRoot.dismiss()
        }
    }

    Timer {
        id: dismissTimer
        interval: 7000
        onTriggered: bubbleRoot.dismiss()
    }

    Timer {
        id: closeTimer
        interval: 320
        onTriggered: bubbleRoot.visible = false
    }
}
