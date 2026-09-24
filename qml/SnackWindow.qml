import QtQuick
import QtQuick.Window

Window {
    id: snackRoot

    objectName: "snackWindow"
    title: "Nala Snacks"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"
    visible: false

    width: 290
    height: 98

    property bool arrowAtTop: false
    property real arrowX: width * 0.5
    readonly property bool isOpen: visible && snackCard.opacity > 0.05

    function appear() {
        snackCard.opacity = 1.0;
        snackCard.scale = 1.0;
        dismissTimer.restart();
    }

    function dismiss() {
        snackCard.opacity = 0.0;
        snackCard.scale = 0.85;
        closeTimer.restart();
    }

    Item {
        id: snackCard
        anchors.fill: parent
        anchors.margins: 4
        opacity: 0.0
        scale: 0.85

        Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            NumberAnimation { duration: 240; easing.type: Easing.OutBack }
        }

        // Pointer triangle
        Rectangle {
            id: pointerArrow
            width: 14
            height: 14
            rotation: 45
            color: "#ee181825"
            border.color: backend.mascotColor
            border.width: 1.5
            antialiasing: true
            x: snackRoot.arrowX - 7
            y: snackRoot.arrowAtTop ? 1 : parent.height - 15
            z: 0
        }

        // Main card body
        Rectangle {
            anchors.fill: parent
            anchors.topMargin: snackRoot.arrowAtTop ? 6 : 0
            anchors.bottomMargin: snackRoot.arrowAtTop ? 0 : 6
            radius: 16
            color: "#ee181825"
            border.color: backend.mascotColor
            border.width: 1.5
            antialiasing: true
            z: 1

            // Mask inner half of the triangle
            Rectangle {
                x: snackRoot.arrowX - 11
                y: snackRoot.arrowAtTop ? 0 : parent.height - 8
                width: 22
                height: 8
                color: "#ee181825"
                z: 2
            }

            Column {
                anchors.centerIn: parent
                spacing: 7
                z: 3

                // Row 1: Snacks
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 9

                    Repeater {
                        model: [
                            { icon: "🍪", name: "cookie" },
                            { icon: "☕", name: "coffee" },
                            { icon: "🐟", name: "fish" },
                            { icon: "🍩", name: "donut" },
                            { icon: "🍓", name: "strawberry" },
                            { icon: "🍵", name: "tea" }
                        ]

                        delegate: Rectangle {
                            required property var modelData
                            width: 34
                            height: 34
                            radius: 17
                            color: snackHover.hovered ? "#40ffffff" : "#1affffff"
                            border.color: snackHover.hovered ? backend.mascotColor : "transparent"
                            border.width: 1.5
                            scale: snackHover.pressed ? 0.88 : (snackHover.hovered ? 1.15 : 1.0)

                            Behavior on color { ColorAnimation { duration: 120 } }
                            Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutBack } }

                            Text {
                                anchors.centerIn: parent
                                text: modelData.icon
                                font.pixelSize: 18
                            }

                            MouseArea {
                                id: snackHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    backend.feed(modelData.name);
                                    snackRoot.dismiss();
                                }
                            }
                        }
                    }
                }

                // Row 2: Toys & Interactions
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 10

                    Repeater {
                        model: [
                            { icon: "🔴", name: "laser", active: backend.laserActive, label: backend.language === "en" ? "Laser" : "Laser" },
                            { icon: "🧶", name: "yarn", active: backend.yarnActive, label: backend.language === "en" ? "Yarn" : "Pelote" },
                            { icon: "🙈", name: "peekaboo", active: backend.peekabooActive, label: backend.language === "en" ? "Peek" : "Coucou" },
                            { icon: "🎉", name: "celebrate", active: false, label: backend.language === "en" ? "Party" : "Fête" }
                        ]

                        delegate: Rectangle {
                            required property var modelData
                            width: 54
                            height: 30
                            radius: 12
                            color: modelData.active ? "#40ff007f" : (toyHover.hovered ? "#35ffffff" : "#16ffffff")
                            border.color: modelData.active ? "#ff007f" : (toyHover.hovered ? backend.mascotColor : "transparent")
                            border.width: 1.5
                            scale: toyHover.pressed ? 0.90 : (toyHover.hovered ? 1.08 : 1.0)

                            Behavior on color { ColorAnimation { duration: 120 } }
                            Behavior on scale { NumberAnimation { duration: 140; easing.type: Easing.OutBack } }

                            Row {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: modelData.icon
                                    font.pixelSize: 15
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                                Text {
                                    text: modelData.label
                                    font.pixelSize: 11
                                    font.bold: true
                                    color: "#f5f5f7"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                id: toyHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (modelData.name === "laser") {
                                        backend.toggleLaser();
                                    } else if (modelData.name === "yarn") {
                                        backend.toggleYarn();
                                    } else if (modelData.name === "peekaboo") {
                                        backend.triggerPeekaboo();
                                    } else if (modelData.name === "celebrate") {
                                        backend.celebrate();
                                    }
                                    snackRoot.dismiss();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Timer {
        id: dismissTimer
        interval: 6000
        onTriggered: snackRoot.dismiss()
    }

    Timer {
        id: closeTimer
        interval: 280
        onTriggered: snackRoot.visible = false
    }
}
