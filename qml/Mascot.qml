import QtQuick
import QtQuick.Window
import Nala

Window {
    id: root

    // The body occupies the middle 1/1.92 of the window; the rest is headroom
    // for morphs and orbit rings that reach past the idle silhouette.
    readonly property real canvasToBody: 1.89
    property bool ready: false

    objectName: "mascotWindow"
    title: "Nala"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"
    // Shown by main() once the surface role has been set up: LayerShellQt can
    // only convert a window before its platform surface is created.
    visible: false

    // One clock for the whole companion: the behaviour model and the orbit
    // simulation advance from the same frame callback, so they never drift.
    FrameAnimation {
        // The self-test drives the clock itself so behaviour is deterministic;
        // a live frame loop would race with it.
        running: !backend.testing
        onTriggered: {
            const dt = Math.min(frameTime, 0.1);
            mascot.tick(dt);
            backend.advance(dt);
            orbits.intensity = mascot.rings;
            orbits.advance(dt);
        }
    }

    // Stepped aside while something is fullscreen. Faded rather than hidden:
    // a layer surface cannot be given its role back once destroyed.
    opacity: backend.outOfTheWay ? 0 : 1
    Behavior on opacity {
        NumberAnimation {
            duration: backend.reducedMotion ? 0 : 260
            easing.type: Easing.InOutCubic
        }
    }

    Item {
        id: stage

        // Square and centred: the mascot's geometry is defined in a unit
        // square, so a non-square window must letterbox rather than stretch.
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width

        // Idle float, drag lean and shake.
        transform: Translate {
            x: mascot.bobX * stage.width * 0.5
            y: mascot.bobY * stage.height * 0.5
        }
        scale: backend.peekabooActive ? 0.88 : 1.0
        Behavior on scale {
            NumberAnimation { duration: 250; easing.type: Easing.OutBack }
        }

        OrbitLayer {
            anchors.fill: parent
            source: orbits
            front: false
            scale2: mascot.bodyScale
        }

        ShaderEffect {
            id: body

            objectName: "bodyShader"
            anchors.fill: parent
            fragmentShader: "qrc:/shaders/mascot.frag.qsb"
            blending: true

            property color bodyColor: backend.mascotColor
            property color bodyColor2: backend.mascotColor2
            property color eyeColor: "#fdfdfd"
            property color badgeColor: "#3ea1f5"
            property real keyPulse: backend.keyPulse
            property real furType: backend.furType
            property real furParam1: 0.0
            property real furParam2: 0.0

            property real formA: mascot.formA
            property real formB: mascot.formB
            property real formMix: mascot.formMix

            property real squashX: mascot.squashX
            property real squashY: mascot.squashY
            property real bodyScale: mascot.bodyScale
            property vector4d bodyTransform: mascot.bodyTransform

            property vector2d eyeLeft: Qt.vector2d(mascot.eyeLeftX, mascot.eyeLeftY)
            property vector2d eyeRight: Qt.vector2d(mascot.eyeRightX, mascot.eyeRightY)
            property vector2d eyeLeftScale: Qt.vector2d(mascot.eyeLeftScaleX, mascot.eyeLeftScaleY)
            property vector2d eyeRightScale: Qt.vector2d(mascot.eyeRightScaleX, mascot.eyeRightScaleY)
            property real eyeLeftAngle: mascot.eyeLeftAngle
            property real eyeRightAngle: mascot.eyeRightAngle
            property real eyeWidth: mascot.eyeWidth
            property real eyeHeight: mascot.eyeHeight
            property real eyeRound: mascot.eyeRound

            property real badge: mascot.badge
            property real dotsSpread: mascot.dotsSpread
            property real dotsShrink: mascot.dotsShrink
            property real dotsPhase: mascot.dotsPhase
        }

        OrbitLayer {
            anchors.fill: parent
            source: orbits
            front: true
            scale2: mascot.bodyScale
        }

        Trail {
            anchors.fill: parent
            angle: mascot.dashAngle
            length: mascot.dashLength
            intensity: mascot.dashIntensity
            z: -1
        }

        // Keyboard reaction pulse ring
        Rectangle {
            anchors.centerIn: stage
            width: stage.width * (0.65 + (1.0 - backend.keyPulse) * 0.45)
            height: width
            radius: width * 0.5
            color: "transparent"
            border.color: backend.mascotColor
            border.width: 2.5
            opacity: backend.keyPulse * 0.85
            visible: backend.keyPulse > 0.01
            z: -2
        }

        // Droplets thrown clear when she comes apart.
        Repeater {
            model: mascot.droplets

            delegate: Rectangle {
                required property var modelData

                readonly property real unit: stage.width * 0.5 * 0.529

                color: backend.mascotColor
                opacity: modelData.opacity
                width: Math.max(1, modelData.radius * 2 * unit)
                height: width
                radius: width / 2
                antialiasing: true
                x: stage.width / 2 + modelData.x * unit - width / 2
                y: stage.height / 2 - modelData.y * unit - height / 2
            }
        }

        // --- Mochi: Floating Hearts ---
        ListModel {
            id: heartsModel
        }

        Repeater {
            model: heartsModel

            delegate: Text {
                id: heartItem
                required property real birth
                required property real startX
                required property real startY
                required property real targetY
                required property real sway
                required property string heartColor
                required property real size
                required property real rot

                z: 15
                text: "♥"
                font.pixelSize: size
                font.bold: true
                color: heartColor
                x: startX - width / 2
                y: startY - height / 2
                rotation: rot
                opacity: 0.0

                Component.onCompleted: heartAnim.start()

                SequentialAnimation {
                    id: heartAnim
                    ParallelAnimation {
                        NumberAnimation {
                            target: heartItem
                            property: "y"
                            to: targetY
                            duration: 950 + Math.random() * 250
                            easing.type: Easing.OutQuad
                        }
                        SequentialAnimation {
                            NumberAnimation {
                                target: heartItem
                                property: "x"
                                to: startX + sway
                                duration: 450
                                easing.type: Easing.InOutSine
                            }
                            NumberAnimation {
                                target: heartItem
                                property: "x"
                                to: startX - sway * 0.5
                                duration: 550
                                easing.type: Easing.InOutSine
                            }
                        }
                        SequentialAnimation {
                            NumberAnimation {
                                target: heartItem
                                property: "opacity"
                                from: 0.0
                                to: 0.95
                                duration: 150
                            }
                            PauseAnimation { duration: 400 }
                            NumberAnimation {
                                target: heartItem
                                property: "opacity"
                                to: 0.0
                                duration: 450
                            }
                        }
                        NumberAnimation {
                            target: heartItem
                            property: "scale"
                            from: 0.4
                            to: 1.05
                            duration: 250
                            easing.type: Easing.OutBack
                        }
                    }
                    ScriptAction {
                        script: {
                            for (let i = 0; i < heartsModel.count; ++i) {
                                if (heartsModel.get(i).birth === birth) {
                                    heartsModel.remove(i);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- Musical Notes for Singing ---
        ListModel {
            id: notesModel
        }

        Repeater {
            model: notesModel

            delegate: Text {
                id: noteItem
                required property real birth
                required property real startX
                required property real startY
                required property real targetY
                required property real sway
                required property string noteColor
                required property real size
                required property string noteSymbol
                required property real rot

                z: 24
                text: noteSymbol
                font.pixelSize: size
                font.bold: true
                color: noteColor
                x: startX - width / 2
                y: startY - height / 2
                rotation: rot
                opacity: 0.0

                Component.onCompleted: noteAnim.start()

                SequentialAnimation {
                    id: noteAnim
                    ParallelAnimation {
                        NumberAnimation {
                            target: noteItem
                            property: "y"
                            to: targetY
                            duration: 1100 + Math.random() * 200
                            easing.type: Easing.OutQuad
                        }
                        SequentialAnimation {
                            NumberAnimation {
                                target: noteItem
                                property: "x"
                                to: startX + sway
                                duration: 450
                                easing.type: Easing.InOutSine
                            }
                            NumberAnimation {
                                target: noteItem
                                property: "x"
                                to: startX - sway * 0.6
                                duration: 650
                                easing.type: Easing.InOutSine
                            }
                        }
                        SequentialAnimation {
                            NumberAnimation {
                                target: noteItem
                                property: "opacity"
                                from: 0.0
                                to: 0.95
                                duration: 160
                            }
                            PauseAnimation { duration: 320 }
                            NumberAnimation {
                                target: noteItem
                                property: "opacity"
                                to: 0.0
                                duration: 520
                                easing.type: Easing.InQuad
                            }
                        }
                        NumberAnimation {
                            target: noteItem
                            property: "scale"
                            from: 0.4
                            to: 1.12
                            duration: 280
                            easing.type: Easing.OutBack
                        }
                    }
                    ScriptAction {
                        script: {
                            for (let i = 0; i < notesModel.count; ++i) {
                                if (notesModel.get(i).birth === birth) {
                                    notesModel.remove(i);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- Celebration Confetti ---
        ListModel {
            id: confettiModel
        }

        Repeater {
            model: confettiModel

            delegate: Rectangle {
                id: confItem
                required property real birth
                required property real startX
                required property real startY
                required property real targetX
                required property real targetY
                required property string confColor
                required property real cWidth
                required property real cHeight
                required property real rot
                required property real targetRot

                width: cWidth
                height: cHeight
                radius: 2
                color: confColor
                x: startX - width / 2
                y: startY - height / 2
                rotation: rot
                z: 32
                opacity: 0.0

                Component.onCompleted: confAnim.start()

                SequentialAnimation {
                    id: confAnim
                    ParallelAnimation {
                        NumberAnimation { target: confItem; property: "x"; to: targetX; duration: 750; easing.type: Easing.OutCubic }
                        NumberAnimation { target: confItem; property: "y"; to: targetY; duration: 750; easing.type: Easing.InQuad }
                        NumberAnimation { target: confItem; property: "rotation"; to: targetRot; duration: 750 }
                        SequentialAnimation {
                            NumberAnimation { target: confItem; property: "opacity"; from: 0.0; to: 1.0; duration: 100 }
                            PauseAnimation { duration: 250 }
                            NumberAnimation { target: confItem; property: "opacity"; to: 0.0; duration: 400; easing.type: Easing.InQuad }
                        }
                    }
                    ScriptAction {
                        script: {
                            for (let i = 0; i < confettiModel.count; ++i) {
                                if (confettiModel.get(i).birth === birth) {
                                    confettiModel.remove(i);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // --- Dizzy Orbiting Cartoon Stars ---
        Repeater {
            model: 3

            delegate: Text {
                id: dizzyStar
                required property int index
                readonly property real starAngle: (mascot.dizzyAngle * 3.4) + index * (Math.PI * 2 / 3)
                readonly property real orbitRx: stage.width * 0.28
                readonly property real orbitRy: stage.height * 0.09
                readonly property real orbitCx: stage.width * 0.5
                readonly property real orbitCy: stage.height * 0.32

                visible: mascot.dizzy
                z: Math.sin(starAngle) > 0 ? 25 : 5
                text: "⭐"
                font.pixelSize: 19
                color: "#ffd166"
                scale: 0.85 + 0.35 * Math.sin(starAngle)
                opacity: mascot.dizzy ? 0.95 : 0.0
                x: orbitCx + Math.cos(starAngle) * orbitRx - width * 0.5
                y: orbitCy + Math.sin(starAngle) * orbitRy - height * 0.5
                rotation: mascot.dizzyAngle * 120 + index * 45
            }
        }

        // --- Mochi: Crumb & Sparkle Particles ---
        ListModel {
            id: crumbsModel
        }

        Repeater {
            model: crumbsModel

            delegate: Text {
                id: crumbItem
                required property string symbol
                required property real startX
                required property real startY
                required property real vx
                required property real vy
                required property string pColor
                required property real pSize

                text: symbol
                font.pixelSize: pSize
                color: pColor
                x: startX - width / 2
                y: startY - height / 2
                z: 28
                opacity: 1.0

                Component.onCompleted: crumbAnim.start()

                ParallelAnimation {
                    id: crumbAnim
                    NumberAnimation { target: crumbItem; property: "x"; to: startX + vx; duration: 440; easing.type: Easing.OutQuad }
                    NumberAnimation { target: crumbItem; property: "y"; to: startY + vy; duration: 440; easing.type: Easing.OutQuad }
                    NumberAnimation { target: crumbItem; property: "opacity"; from: 1.0; to: 0.0; duration: 440; easing.type: Easing.InQuad }
                    NumberAnimation { target: crumbItem; property: "scale"; from: 1.3; to: 0.3; duration: 440 }
                }
            }
        }

        // --- Accessories & Cosmetics Layer ---
        Item {
            id: accessoryLayer
            anchors.fill: parent
            z: 22
            visible: backend.accessory !== "none"

            transform: [
                Scale {
                    xScale: mascot.bodyScale * mascot.squashX
                    yScale: mascot.bodyScale * mascot.squashY
                    origin.x: stage.width * 0.5
                    origin.y: stage.height * 0.5
                },
                Rotation {
                    angle: mascot.roll * 180 / Math.PI + (mascot.dizzy ? Math.sin(mascot.dizzyAngle * 4) * 14 : 0)
                    origin.x: stage.width * 0.5
                    origin.y: stage.height * 0.5
                }
            ]

            // 1. HEADPHONES 🎧
            Item {
                id: headphonesAcc
                anchors.fill: parent
                visible: backend.accessory === "headphones"

                // Headband arch
                Item {
                    x: parent.width * 0.5 - 46
                    y: parent.height * 0.25
                    width: 92
                    height: 28
                    clip: true

                    Rectangle {
                        x: 0
                        y: 0
                        width: 92
                        height: 52
                        radius: 46
                        color: "transparent"
                        border.width: 5
                        border.color: "#282a36"
                        antialiasing: true
                    }
                }

                // Left Earcup
                Rectangle {
                    x: parent.width * 0.5 - 54
                    y: parent.height * 0.38
                    width: 17
                    height: 33
                    radius: 8.5
                    color: "#181825"
                    border.width: 2.2
                    border.color: backend.musicPlaying ? "#00f2fe" : "#bd93f9"
                    antialiasing: true

                    // Pulse glow
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width + 8
                        height: parent.height + 8
                        radius: 12
                        color: "transparent"
                        border.width: 2
                        border.color: "#00f2fe"
                        opacity: backend.musicPlaying ? 0.75 : 0.0
                        visible: backend.musicPlaying
                    }

                    // Mini Equalizer bars
                    Row {
                        anchors.centerIn: parent
                        spacing: 2
                        visible: backend.musicPlaying

                        Repeater {
                            model: 3
                            delegate: Rectangle {
                                required property int index
                                width: 2.5
                                height: 6 + Math.abs(Math.sin(mascot.time * 8 + index * 1.5)) * 14
                                radius: 1
                                color: index === 0 ? "#00f2fe" : index === 1 ? "#ec4899" : "#ffd166"
                            }
                        }
                    }
                }

                // Right Earcup
                Rectangle {
                    x: parent.width * 0.5 + 37
                    y: parent.height * 0.38
                    width: 17
                    height: 33
                    radius: 8.5
                    color: "#181825"
                    border.width: 2.2
                    border.color: backend.musicPlaying ? "#00f2fe" : "#bd93f9"
                    antialiasing: true

                    // Pulse glow
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width + 8
                        height: parent.height + 8
                        radius: 12
                        color: "transparent"
                        border.width: 2
                        border.color: "#00f2fe"
                        opacity: backend.musicPlaying ? 0.75 : 0.0
                        visible: backend.musicPlaying
                    }

                    // Mini Equalizer bars
                    Row {
                        anchors.centerIn: parent
                        spacing: 2
                        visible: backend.musicPlaying

                        Repeater {
                            model: 3
                            delegate: Rectangle {
                                required property int index
                                width: 2.5
                                height: 6 + Math.abs(Math.sin(mascot.time * 8 + (index + 1) * 1.8)) * 14
                                radius: 1
                                color: index === 0 ? "#ffd166" : index === 1 ? "#ec4899" : "#00f2fe"
                            }
                        }
                    }
                }
            }

            // 2. SUNGLASSES 😎
            Item {
                id: sunglassesAcc
                anchors.fill: parent
                visible: backend.accessory === "sunglasses"

                readonly property real eyeCenterY: parent.height * 0.5 - (mascot.eyeLeftY + mascot.eyeRightY) * 0.5 * (parent.width * 0.5 * 0.529)

                // Left Lens
                Rectangle {
                    x: parent.width * 0.5 - 35
                    y: sunglassesAcc.eyeCenterY - 11
                    width: 31
                    height: 20
                    radius: 6
                    color: "#111116"
                    border.width: 1.5
                    border.color: "#313244"
                    clip: true
                    antialiasing: true

                    Rectangle {
                        x: 8
                        y: -3
                        width: 4
                        height: 28
                        rotation: 30
                        color: "#60ffffff"
                    }
                }

                // Right Lens
                Rectangle {
                    x: parent.width * 0.5 + 4
                    y: sunglassesAcc.eyeCenterY - 11
                    width: 31
                    height: 20
                    radius: 6
                    color: "#111116"
                    border.width: 1.5
                    border.color: "#313244"
                    clip: true
                    antialiasing: true

                    Rectangle {
                        x: 8
                        y: -3
                        width: 4
                        height: 28
                        rotation: 30
                        color: "#60ffffff"
                    }
                }

                // Bridge
                Rectangle {
                    x: parent.width * 0.5 - 5
                    y: sunglassesAcc.eyeCenterY - 5
                    width: 10
                    height: 3
                    radius: 1.5
                    color: "#313244"
                }
            }

            // 3. ROUND GLASSES 👓
            Item {
                id: glassesAcc
                anchors.fill: parent
                visible: backend.accessory === "glasses"

                readonly property real eyeCenterY: parent.height * 0.5 - (mascot.eyeLeftY + mascot.eyeRightY) * 0.5 * (parent.width * 0.5 * 0.529)

                // Left Rim
                Rectangle {
                    x: parent.width * 0.5 - 34
                    y: glassesAcc.eyeCenterY - 13
                    width: 26
                    height: 26
                    radius: 13
                    color: "#12ffffff"
                    border.width: 2.2
                    border.color: "#f59e0b"
                    antialiasing: true

                    Rectangle {
                        x: 14
                        y: 3
                        width: 3
                        height: 9
                        rotation: 25
                        color: "#70ffffff"
                    }
                }

                // Right Rim
                Rectangle {
                    x: parent.width * 0.5 + 8
                    y: glassesAcc.eyeCenterY - 13
                    width: 26
                    height: 26
                    radius: 13
                    color: "#12ffffff"
                    border.width: 2.2
                    border.color: "#f59e0b"
                    antialiasing: true

                    Rectangle {
                        x: 14
                        y: 3
                        width: 3
                        height: 9
                        rotation: 25
                        color: "#70ffffff"
                    }
                }

                // Nose Bridge
                Rectangle {
                    x: parent.width * 0.5 - 8
                    y: glassesAcc.eyeCenterY
                    width: 16
                    height: 2
                    color: "#f59e0b"
                    radius: 1
                }
            }

            // 4. ROYAL CROWN 👑
            Item {
                id: crownAcc
                anchors.fill: parent
                visible: backend.accessory === "crown"

                Item {
                    x: parent.width * 0.5 - 28
                    y: parent.height * 0.20
                    width: 56
                    height: 26

                    Rectangle {
                        y: 18
                        width: 56
                        height: 7
                        radius: 3
                        color: "#f59e0b"
                        border.color: "#ffd166"
                        border.width: 1
                    }

                    Rectangle {
                        x: 2
                        y: 4
                        width: 14
                        height: 18
                        rotation: -18
                        color: "#f59e0b"
                        border.color: "#ffd166"
                        border.width: 1
                    }

                    Rectangle {
                        x: 21
                        y: 0
                        width: 14
                        height: 22
                        color: "#fbbf24"
                        border.color: "#fef08a"
                        border.width: 1
                    }

                    Rectangle {
                        x: 40
                        y: 4
                        width: 14
                        height: 18
                        rotation: 18
                        color: "#f59e0b"
                        border.color: "#ffd166"
                        border.width: 1
                    }

                    // Jewels
                    Rectangle { x: 3; y: 1; width: 6; height: 6; radius: 3; color: "#ef4444" }
                    Rectangle { x: 25; y: -3; width: 6; height: 6; radius: 3; color: "#3b82f6" }
                    Rectangle { x: 47; y: 1; width: 6; height: 6; radius: 3; color: "#10b981" }
                }
            }

            // 5. FLOWER 🌸
            Item {
                id: flowerAcc
                anchors.fill: parent
                visible: backend.accessory === "flower"

                Item {
                    x: parent.width * 0.5 - 46
                    y: parent.height * 0.28
                    width: 28
                    height: 28

                    Repeater {
                        model: 5
                        delegate: Rectangle {
                            required property int index
                            x: 14 - 6
                            y: 14 - 12
                            width: 12
                            height: 16
                            radius: 6
                            color: "#ff758f"
                            transform: Rotation {
                                angle: index * 72
                                origin.x: 6
                                origin.y: 12
                            }
                        }
                    }

                    Rectangle {
                        x: 14 - 5
                        y: 14 - 5
                        width: 10
                        height: 10
                        radius: 5
                        color: "#ffd166"
                        border.color: "#f59e0b"
                        border.width: 1
                    }
                }
            }

            // 6. NIGHTCAP 💤
            Item {
                id: nightcapAcc
                anchors.fill: parent
                visible: backend.accessory === "nightcap"

                Item {
                    // Positioned cleanly atop Nala's head, comfortably above her eyes
                    x: parent.width * 0.5 - 28
                    y: parent.height * 0.14
                    width: 58
                    height: 38

                    // Main cozy slouchy hood in indigo/violet
                    Rectangle {
                        x: 8
                        y: 8
                        width: 38
                        height: 20
                        radius: 10
                        color: "#4f46e5"
                        rotation: -4
                    }

                    // Slouch curving over to the right
                    Rectangle {
                        x: 22
                        y: 0
                        width: 26
                        height: 18
                        radius: 8
                        color: "#6366f1"
                        rotation: 30
                    }

                    // Hanging tail
                    Rectangle {
                        x: 36
                        y: 8
                        width: 16
                        height: 16
                        radius: 7
                        color: "#818cf8"
                        rotation: 60
                    }

                    // Cute golden embroidered star
                    Text {
                        x: 20
                        y: 8
                        text: "✦"
                        font.pixelSize: 8
                        color: "#fef08a"
                        opacity: 0.95
                    }

                    // Fluffy white brim on forehead (well above eyes)
                    Rectangle {
                        x: 2
                        y: 20
                        width: 46
                        height: 10
                        radius: 5
                        color: "#ffffff"
                        border.color: "#e2e8f0"
                        border.width: 1
                        rotation: -5
                    }

                    // Fluffy white pom-pom bobbing gently with breathing on right tip
                    Rectangle {
                        x: 44
                        y: 18 + Math.sin(mascot.time * 3.5) * 1.5
                        width: 13
                        height: 13
                        radius: 6.5
                        color: "#ffffff"
                        border.color: "#e2e8f0"
                        border.width: 1
                    }
                }
            }
        }


        // --- Mochi: Snack Animation ---
        Item {
            id: snackAnimation
            z: 27
            anchors.horizontalCenter: stage.horizontalCenter
            anchors.verticalCenter: stage.verticalCenter
            anchors.horizontalCenterOffset: xOffset
            anchors.verticalCenterOffset: yOffset
            property real sideSign: 1
            property real xOffset: 0
            property real yOffset: 0
            width: 38
            height: 38
            visible: opacity > 0.01
            opacity: 0.0
            scale: 0.0
            rotation: 0

            property string currentSnackIcon: "🍪"
            property string currentSnackType: "cookie"

            Text {
                anchors.centerIn: parent
                text: snackAnimation.currentSnackIcon
                font.pixelSize: 32
            }

            SequentialAnimation {
                id: chewAnim

                // 1. Pop in from the side she's gazing towards
                ParallelAnimation {
                    NumberAnimation { target: snackAnimation; property: "opacity"; from: 0.0; to: 1.0; duration: 180; easing.type: Easing.OutCubic }
                    NumberAnimation { target: snackAnimation; property: "scale"; from: 0.3; to: 1.25; duration: 250; easing.type: Easing.OutBack }
                    NumberAnimation { target: snackAnimation; property: "xOffset"; from: snackAnimation.sideSign * 58; to: snackAnimation.sideSign * 42; duration: 250; easing.type: Easing.OutQuad }
                    NumberAnimation { target: snackAnimation; property: "yOffset"; from: 14; to: 4; duration: 250; easing.type: Easing.OutQuad }
                }
                PauseAnimation { duration: 150 }

                // 2. First bite! Crunch!
                ScriptAction {
                    script: {
                        const bx = stage.width * 0.5 + snackAnimation.xOffset;
                        const by = stage.height * 0.5 + snackAnimation.yOffset;
                        spawnCrumbs(bx, by, 4);
                        spawnHeart(bx, by - 12, 0.75);
                    }
                }
                ParallelAnimation {
                    NumberAnimation { target: snackAnimation; property: "scale"; to: 0.85; duration: 140; easing.type: Easing.InBack }
                    NumberAnimation { target: snackAnimation; property: "rotation"; to: snackAnimation.sideSign * -16; duration: 140 }
                    NumberAnimation { target: snackAnimation; property: "xOffset"; to: snackAnimation.sideSign * 26; duration: 140 }
                    NumberAnimation { target: snackAnimation; property: "yOffset"; to: 0; duration: 140 }
                }
                PauseAnimation { duration: 110 }

                // 3. Second bite! Crunch!
                ScriptAction {
                    script: {
                        const bx = stage.width * 0.5 + snackAnimation.xOffset;
                        const by = stage.height * 0.5 + snackAnimation.yOffset;
                        spawnCrumbs(bx, by, 5);
                        spawnHeart(bx, by - 14, 0.85);
                    }
                }
                ParallelAnimation {
                    NumberAnimation { target: snackAnimation; property: "scale"; to: 0.45; duration: 140; easing.type: Easing.InBack }
                    NumberAnimation { target: snackAnimation; property: "rotation"; to: snackAnimation.sideSign * 16; duration: 140 }
                    NumberAnimation { target: snackAnimation; property: "xOffset"; to: snackAnimation.sideSign * 12; duration: 140 }
                    NumberAnimation { target: snackAnimation; property: "yOffset"; to: -4; duration: 140 }
                }
                PauseAnimation { duration: 110 }

                // 4. Final gulp!
                ScriptAction {
                    script: {
                        const bx = stage.width * 0.5 + snackAnimation.xOffset;
                        const by = stage.height * 0.5 + snackAnimation.yOffset;
                        spawnCrumbs(bx, by, 6);
                    }
                }
                ParallelAnimation {
                    NumberAnimation { target: snackAnimation; property: "scale"; to: 0.0; duration: 150 }
                    NumberAnimation { target: snackAnimation; property: "opacity"; to: 0.0; duration: 150 }
                    NumberAnimation { target: snackAnimation; property: "xOffset"; to: 0; duration: 150 }
                    NumberAnimation { target: snackAnimation; property: "rotation"; to: 0; duration: 150 }
                }

                // 5. Joyful celebration burst!
                ScriptAction {
                    script: {
                        mascot.love();
                        spawnHeartBurst(stage.width * 0.5, stage.height * 0.44);
                    }
                }
            }
        }
    }



    // --- Fatigue Sleepy Zzz Timer (Night / Early Morning) ---
    Timer {
        id: tiredZzzTimer
        interval: 22000 + Math.random() * 14000
        running: backend.timeFatigued && !backend.testing
        repeat: true
        onTriggered: {
            if (mascot.mood === 0 && !backend.flying) { // Resting
                spawnZzz(stage.width * 0.5, stage.height * 0.35);
            }
            interval = 24000 + Math.random() * 18000;
        }
    }

    // --- Singing & Music Timer ---
    Timer {
        id: musicSingTimer
        interval: 10000 + Math.random() * 6000
        running: backend.musicPlaying && !backend.testing
        repeat: true
        onTriggered: {
            singSong();
            interval = 10000 + Math.random() * 8000;
        }
    }

    function spawnMusicNote(cx, cy) {
        if (notesModel.count > 20) return;
        const notes = ["♪", "♫", "♬", "♩"];
        const colors = ["#00f2fe", "#4facfe", "#ff758f", "#a78bfa", "#f59e0b", "#10b981", "#ff007f"];
        const sym = notes[Math.floor(Math.random() * notes.length)];
        const col = colors[Math.floor(Math.random() * colors.length)];
        const x = cx !== undefined ? cx : stage.width * 0.5 + (Math.random() - 0.5) * (stage.width * 0.22);
        const y = cy !== undefined ? cy : stage.height * 0.44;
        // Keep targetY well inside the window bounds so top of notes are never clipped
        const topLimit = Math.max(16, stage.height * 0.14);
        const travel = stage.height * 0.16 + Math.random() * (stage.height * 0.08);
        const targetY = Math.max(topLimit, y - travel);
        const sway = (Math.random() - 0.5) * (stage.width * 0.16);
        const sz = Math.max(11, Math.min(20, stage.height * 0.10 + Math.random() * 4));
        notesModel.append({
            birth: Date.now() + Math.random(),
            startX: x,
            startY: y,
            targetY: targetY,
            sway: sway,
            noteColor: col,
            size: sz,
            noteSymbol: sym,
            rot: (Math.random() - 0.5) * 30
        });
    }

    function singSong() {
        for (let i = 0; i < 4; ++i) {
            spawnMusicNote(stage.width * 0.5 + (i - 1.5) * (stage.width * 0.10), stage.height * 0.44 - (i % 2) * (stage.height * 0.05));
        }
        mascot.pet();
        if (backend.mochiBubbles && Math.random() < 0.45) {
            const frLines = [
                "La la laaa~ 🎶",
                "♪ Mm-mm-mmm~ 🎵",
                "J'adore cette musique ! 🎧✨",
                "♪♫ Tra-la-la~ 🌸",
                "En rythme ! 🐾🎶"
            ];
            const enLines = [
                "La la laaa~ 🎶",
                "♪ Mm-mm-mmm~ 🎵",
                "I love this song! 🎧✨",
                "♪♫ Tra-la-la~ 🌸",
                "To the beat! 🐾🎶"
            ];
            const list = (backend.language === "en") ? enLines : frLines;
            backend.showSpeech(list[Math.floor(Math.random() * list.length)]);
        }
    }

    function spawnConfetti() {
        const colors = ["#ff007f", "#3b82f6", "#10b981", "#f59e0b", "#ec4899", "#8b5cf6", "#ffd166", "#06b6d4"];
        const cx = stage.width * 0.5;
        const cy = stage.height * 0.44;
        for (let i = 0; i < 28; ++i) {
            const angle = Math.random() * Math.PI * 2;
            const dist = stage.width * 0.10 + Math.random() * (stage.width * 0.22);
            const tx = Math.max(10, Math.min(stage.width - 10, cx + Math.cos(angle) * dist));
            const ty = Math.max(10, Math.min(stage.height - 10, cy + Math.sin(angle) * dist * 0.75 + stage.height * 0.08));
            confettiModel.append({
                birth: Date.now() + Math.random(),
                startX: cx + (Math.random() - 0.5) * (stage.width * 0.16),
                startY: cy + (Math.random() - 0.5) * (stage.height * 0.14),
                targetX: tx,
                targetY: ty,
                confColor: colors[Math.floor(Math.random() * colors.length)],
                cWidth: 4 + Math.random() * 4,
                cHeight: 4 + Math.random() * 4,
                rot: Math.random() * 360,
                targetRot: Math.random() * 720
            });
        }
    }

    function spawnSparkles(cx, cy, count) {
        const symbols = ["✨", "⭐", "✦", "💫"];
        const colors = ["#ffd166", "#f59e0b", "#ffbe0b", "#f97316"];
        for (let i = 0; i < (count || 4); ++i) {
            const angle = Math.random() * Math.PI * 2;
            const speed = 20 + Math.random() * 35;
            crumbsModel.append({
                symbol: symbols[Math.floor(Math.random() * symbols.length)],
                startX: cx,
                startY: cy,
                vx: Math.cos(angle) * speed,
                vy: Math.sin(angle) * speed - 15,
                pColor: colors[Math.floor(Math.random() * colors.length)],
                pSize: 13 + Math.random() * 6
            });
        }
    }

    function spawnZzz(cx, cy) {
        const letters = ["z", "Z", "💤"];
        for (let i = 0; i < 3; ++i) {
            crumbsModel.append({
                symbol: letters[i],
                startX: cx + (i - 1) * 14,
                startY: cy - i * 12,
                vx: 12 + i * 8,
                vy: -35 - i * 15,
                pColor: "#a5b4fc",
                pSize: 12 + i * 4
            });
        }
    }

    function spawnCrumbs(cx, cy, count) {
        const symbols = ["•", "✦", "✧", "•"];
        const colors = ["#ffbe0b", "#fb5607", "#ff006e", "#8338ec", "#3a86ff", "#ffd166"];
        for (let i = 0; i < (count || 5); ++i) {
            const angle = Math.random() * Math.PI * 2;
            const speed = 25 + Math.random() * 35;
            crumbsModel.append({
                symbol: Math.random() > 0.4 ? "✦" : "•",
                startX: cx,
                startY: cy,
                vx: Math.cos(angle) * speed,
                vy: Math.sin(angle) * speed + 8,
                pColor: colors[Math.floor(Math.random() * colors.length)],
                pSize: 10 + Math.random() * 8
            });
        }
    }

    function spawnHeart(cx, cy, scaleMul) {
        if (heartsModel.count > 25) return;
        const colors = ["#ff6584", "#ff758f", "#ff85a1", "#ff4d6d", "#f72585"];
        const col = colors[Math.floor(Math.random() * colors.length)];
        const targetY = cy - 35 - Math.random() * 30;
        const sway = (Math.random() - 0.5) * 26;
        const sz = (13 + Math.random() * 7) * (scaleMul || 1.0);
        heartsModel.append({
            birth: Date.now() + Math.random(),
            startX: cx + (Math.random() - 0.5) * 20,
            startY: cy + (Math.random() - 0.5) * 10,
            targetY: targetY,
            sway: sway,
            heartColor: col,
            size: sz,
            rot: (Math.random() - 0.5) * 30
        });
    }

    function spawnHeartBurst(cx, cy) {
        const x = cx !== undefined ? cx : stage.width * 0.5;
        const y = cy !== undefined ? cy : stage.height * 0.45;
        for (let i = 0; i < 7; ++i) {
            spawnHeart(x, y, 1.15);
        }
        spawnCrumbs(x, y, 6);
    }

    function feedSnack(snackType) {
        snackAnimation.currentSnackType = snackType;
        if (snackType === "coffee")
            snackAnimation.currentSnackIcon = "☕";
        else if (snackType === "fish")
            snackAnimation.currentSnackIcon = "🐟";
        else if (snackType === "donut")
            snackAnimation.currentSnackIcon = "🍩";
        else if (snackType === "strawberry")
            snackAnimation.currentSnackIcon = "🍓";
        else if (snackType === "tea")
            snackAnimation.currentSnackIcon = "🍵";
        else
            snackAnimation.currentSnackIcon = "🍪";

        const lookRight = (mascot.gazeYaw >= 0);
        snackAnimation.sideSign = lookRight ? 1 : -1;

        mascot.feed(snackType);
        chewAnim.restart();
    }

    MouseArea {
        id: pointer

        anchors.centerIn: parent
        width: parent.width / root.canvasToBody
        height: parent.height / root.canvasToBody

        property point origin: Qt.point(0, 0)
        property bool moved: false
        property real lastX: 0
        property real lastY: 0
        property real petScore: 0
        property real tickleScore: 0
        property real lastHeartSpawnTime: 0
        property real lastDragDx: 0
        property int dragReversals: 0
        property real lastReversalTime: 0
        property bool wasShaken: false
        property var clickHistory: []
        property real lastPancakeTime: 0
        property real lastTickleTime: 0

        // Rub / tickle detection properties
        property real lastRubDx: 0
        property real lastRubDy: 0
        property int rubDirectionChanges: 0
        property real lastRubTime: 0
        property real tickleActiveUntil: 0

        function handlePetHover(x, y) {
            const now = Date.now();
            if (lastX === 0 && lastY === 0) {
                lastX = x;
                lastY = y;
                return;
            }
            const dx = x - lastX;
            const dy = y - lastY;
            const dist = Math.sqrt(dx * dx + dy * dy);
            lastX = x;
            lastY = y;

            if (dist < 1.0)
                return;

            // Detect rapid back-and-forth scrubbing (direction changes)
            const isHorizontalRub = (dx > 2.5 && lastRubDx < -2.5) || (dx < -2.5 && lastRubDx > 2.5);
            const isVerticalRub = (dy > 2.5 && lastRubDy < -2.5) || (dy < -2.5 && lastRubDy > 2.5);

            if (isHorizontalRub || isVerticalRub) {
                if (now - lastRubTime < 350) {
                    rubDirectionChanges++;
                } else {
                    rubDirectionChanges = 1;
                }
                lastRubTime = now;
            }

            if (Math.abs(dx) > 2.5)
                lastRubDx = dx;
            if (Math.abs(dy) > 2.5)
                lastRubDy = dy;

            // If user has reversed directions rapidly, or is already in active tickle state
            const isRapidRubbing = (rubDirectionChanges >= 2 && (now - lastRubTime < 450));
            const isTickling = isRapidRubbing || (now < tickleActiveUntil);

            if (isTickling) {
                tickleActiveUntil = now + 450; // Keep tickle state active during ongoing scrubbing
                petScore = 0; // Suppress hearts completely!
                tickleScore += dist * 1.2;

                if (tickleScore >= 36 && (now - lastTickleTime > 650)) {
                    tickleScore = 0;
                    lastTickleTime = now;
                    backend.tickle();
                }
                return;
            }

            // High-five trigger on paw area (Level >= 2)
            if (backend.level >= 2 && x > pointer.width * 0.75 && y < pointer.height * 0.40) {
                if (now - lastHeartSpawnTime > 900) {
                    lastHeartSpawnTime = now;
                    backend.highFive();
                }
                return;
            }

            // Normal gentle caressing
            if (dist > 1.2 && dist < 50) {
                petScore = Math.min(100, petScore + dist * 0.85);
                if (petScore > 32) {
                    mascot.pet();
                    if (now - lastHeartSpawnTime > 280) {
                        lastHeartSpawnTime = now;
                        spawnHeart(stage.width * 0.5, stage.height * 0.45, 0.85);
                    }
                }
            }
        }

        enabled: !backend.testing
        hoverEnabled: !backend.testing
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.PointingHandCursor

        onEntered: {
            mascot.setHovered(true);
            lastX = 0;
            lastY = 0;
            if (backend.peekabooActive) {
                backend.setPeekabooActive(false);
                mascot.wake();
                mascot.love();
                spawnHeartBurst(stage.width * 0.5, stage.height * 0.35);
                spawnSparkles(stage.width * 0.5, stage.height * 0.35, 8);
                spawnConfetti();
                backend.showSpeech(backend.language === "en" ? "Peek-a-boo ! Here I am ! 🌸✨" : "Coucou ! Me voilà ! 🌸✨");
            }
        }
        onExited: {
            mascot.setHovered(false);
            petScore = 0;
            tickleScore = 0;
            rubDirectionChanges = 0;
            tickleActiveUntil = 0;
        }

        onPressed: function (mouse) {
            if (mouse.button === Qt.RightButton) {
                backend.openSettings();
                return;
            }
            if (mouse.button === Qt.MiddleButton) {
                backend.toggleSnackMenu();
                return;
            }

            if (backend.peekabooActive) {
                backend.setPeekabooActive(false);
                mascot.wake();
                mascot.love();
                spawnHeartBurst(stage.width * 0.5, stage.height * 0.35);
                spawnSparkles(stage.width * 0.5, stage.height * 0.35, 8);
                spawnConfetti();
                backend.showSpeech(backend.language === "en" ? "Peek-a-boo ! Here I am ! 🌸✨" : "Coucou ! Me voilà ! 🌸✨");
                return;
            }

            if (mouse.button === Qt.LeftButton) {
                const now = Date.now();
                clickHistory = clickHistory.filter(t => now - t < 850);
                clickHistory.push(now);

                if (clickHistory.length >= 3) {
                    clickHistory = [];
                    lastPancakeTime = now;
                    dblClickTimer.stop();
                    mascot.pancakeSquash();
                    backend.showSpeech(backend.language === "en" ? "Splat! I'm a pancake! 🥞" : "Plaf ! Je suis une crêpe ! 🥞");
                    return;
                }
            }

            origin = Qt.point(mouse.x, mouse.y);
            moved = false;
            lastDragDx = 0;
            dragReversals = 0;
            wasShaken = false;
            backend.grabDrag();
        }

        onPositionChanged: function (mouse) {
            if (backend.dragging()) {
                const dx = mouse.x - origin.x;
                const dy = mouse.y - origin.y;
                if (!moved && Math.abs(dx) + Math.abs(dy) < 9)
                    return;
                moved = true;
                clickHistory = [];
                dblClickTimer.stop();

                // Detect rapid direction reversals during drag (shaking)
                if (Math.abs(dx - lastDragDx) > 18) {
                    const now = Date.now();
                    if ((dx > 0 && lastDragDx < 0) || (dx < 0 && lastDragDx > 0)) {
                        if (now - lastReversalTime < 450) {
                            dragReversals++;
                            if (dragReversals >= 4) {
                                wasShaken = true;
                            }
                        } else {
                            dragReversals = 1;
                        }
                        lastReversalTime = now;
                    }
                }
                lastDragDx = dx;

                backend.dragBy(dx, dy);
                return;
            }

            if (backend.mochiPetting && !pressed) {
                handlePetHover(mouse.x, mouse.y);
            }
        }

        onReleased: function (mouse) {
            if (mouse.button !== Qt.LeftButton)
                return;
            backend.releaseDrag();

            if (wasShaken) {
                wasShaken = false;
                dragReversals = 0;
                mascot.dizzy();
                backend.showSpeech(backend.language === "en" ? "Whoaaa... Everything is spinning! 😵‍💫" : "Whoaaa... Tout tourne ! 😵‍💫");
                return;
            }

            if (!moved) {
                const now = Date.now();
                if (now - lastPancakeTime < 700)
                    return;
                mascot.poke();
            }
        }

        onCanceled: backend.releaseDrag()

        onDoubleClicked: {
            const now = Date.now();
            if (now - lastPancakeTime < 850)
                return;
            dblClickTimer.restart();
        }

        onWheel: function (wheel) {
            mascot.wake();
            if (wheel.angleDelta.y > 0) {
                mascot.stretchPurr();
                spawnSparkles(stage.width * 0.5, stage.height * 0.38, 4);
            } else if (wheel.angleDelta.y < 0) {
                mascot.sleepLull();
                spawnZzz(stage.width * 0.5, stage.height * 0.35);
            }
        }
    }

    Timer {
        id: dblClickTimer
        interval: 220
        repeat: false
        onTriggered: {
            if (Date.now() - pointer.lastPancakeTime < 850)
                return;
            if (backend.level >= 4) {
                backend.backflip();
            } else if (backend.mochiPetting) {
                mascot.love();
                spawnHeartBurst(stage.width * 0.5, stage.height * 0.45);
            } else {
                mascot.think(3.4);
            }
        }
    }

    Timer {
        interval: 100
        repeat: true
        running: !backend.testing
        onTriggered: {
            if (pointer.petScore > 0)
                pointer.petScore = Math.max(0, pointer.petScore - 12);
            if (pointer.tickleScore > 0)
                pointer.tickleScore = Math.max(0, pointer.tickleScore - 15);
            if (Date.now() - pointer.lastRubTime > 400)
                pointer.rubDirectionChanges = 0;
        }
    }

    Loader {
        id: settings

        active: false
        source: "qrc:/qml/Settings.qml"
        onLoaded: {
            item.visible = true;
            item.show();
            item.raise();
            item.requestActivate();
        }
    }

    Connections {
        target: backend
        function onSettingsRequested() {
            if (!settings.active) {
                settings.active = true;
            } else if (settings.item) {
                settings.item.visible = true;
                settings.item.show();
                settings.item.raise();
                settings.item.requestActivate();
            }
        }
        function onSnackRequested(snack) {
            feedSnack(snack);
        }
        function onCelebrateTriggered() {
            spawnConfetti();
        }
        function onMusicPlayingChanged() {
            if (backend.musicPlaying) {
                singSong();
            }
        }
        function onLeveledUp(newLevel, rewardTitle, rewardDesc) {
            spawnConfetti();
            spawnSparkles(stage.width * 0.5, stage.height * 0.45, 20);
        }
    }

    Connections {
        target: mascot
        function onLoved() {
            spawnHeartBurst(stage.width * 0.5, stage.height * 0.45);
        }
        function onCelebrated() {
            spawnConfetti();
        }
        function onTickled() {
            spawnSparkles(stage.width * 0.5, stage.height * 0.44, 6);
        }
        function onPurred() {
            spawnSparkles(stage.width * 0.5, stage.height * 0.38, 4);
        }
        function onBackflipped() {
            spawnSparkles(stage.width * 0.5, stage.height * 0.45, 12);
        }
        function onHighFived() {
            spawnSparkles(stage.width * 0.72, stage.height * 0.35, 8);
            spawnHeart(stage.width * 0.72, stage.height * 0.3, 0.9);
        }
        function onStarShowerTriggered() {
            spawnSparkles(stage.width * 0.5, stage.height * 0.3, 24);
            spawnConfetti();
        }
    }
}
