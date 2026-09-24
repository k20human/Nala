import QtQuick
import QtQuick.Window
import Nala

Window {
    id: toyRoot

    width: 72
    height: 72
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | (backend.laserActive ? Qt.WindowTransparentForInput : 0)

    visible: false

    Item {
        anchors.fill: parent

        // --- Laser Dot (🔴) ---
        Item {
            id: laserItem
            anchors.centerIn: parent
            visible: backend.laserActive
            width: 48
            height: 48

            // Outer pulsing glow
            Rectangle {
                anchors.centerIn: parent
                width: 32 + Math.sin(mascot.time * 14) * 8
                height: width
                radius: width * 0.5
                color: "#35ff0055"
                border.color: "#80ff0055"
                border.width: 1.5
            }

            // Middle bright halo
            Rectangle {
                anchors.centerIn: parent
                width: 18 + Math.sin(mascot.time * 20) * 3
                height: width
                radius: width * 0.5
                color: "#90ff0044"
                border.color: "#ff3366"
                border.width: 1.5
            }

            // Intense core
            Rectangle {
                anchors.centerIn: parent
                width: 8
                height: 8
                radius: 4
                color: "#ffffff"
            }
        }

        // --- Yarn Ball (🧶) ---
        Item {
            id: yarnItem
            anchors.centerIn: parent
            visible: backend.yarnActive
            width: 56
            height: 56

            // Loose trailing thread
            Rectangle {
                x: 6
                y: 38
                width: 18
                height: 3
                radius: 1.5
                color: "#ec4899"
                rotation: -backend.toyRot * 0.35
                transformOrigin: Item.TopLeft
            }

            // Main wool ball
            Rectangle {
                id: ballBody
                anchors.centerIn: parent
                width: 48
                height: 48
                radius: 24
                color: "#db2777"
                border.color: "#f472b6"
                border.width: 2.5
                rotation: backend.toyRot
                antialiasing: true

                // Texture bands
                Rectangle {
                    x: 8
                    y: 11
                    width: 32
                    height: 10
                    radius: 5
                    color: "transparent"
                    border.color: "#fbcfe8"
                    border.width: 2
                    rotation: 32
                }
                Rectangle {
                    x: 10
                    y: 25
                    width: 28
                    height: 10
                    radius: 5
                    color: "transparent"
                    border.color: "#f472b6"
                    border.width: 2
                    rotation: -38
                }
                Rectangle {
                    x: 14
                    y: 18
                    width: 20
                    height: 8
                    radius: 4
                    color: "transparent"
                    border.color: "#ffffff"
                    opacity: 0.6
                    border.width: 1.5
                    rotation: 12
                }
            }

            // Clickable area to bat / kick the yarn ball across the desktop
            MouseArea {
                anchors.fill: parent
                enabled: backend.yarnActive
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    backend.toyClicked();
                }
            }
        }
    }
}
