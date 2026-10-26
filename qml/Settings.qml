import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: settings

    readonly property int motion: backend.reducedMotion ? 0 : 120 * theme.motionScale
    readonly property real controlRadius: 12 * theme.radius
    readonly property real cardRadius: 16 * theme.radius

    readonly property var i18n: ({
        "fr": {
            "title": "Nala",
            "close": "Fermer les paramètres",
            "size": "Taille",
            "color": "Couleur",
            "color_ink": "Encre",
            "color_theme": "Fond d'écran",
            "color_custom": "Personnalisé",
            "color_gradient": "Dégradé",
            "color_fur": "Fourrure",
            "fur_calico": "Calico 🐱",
            "fur_tabby": "Tigré 🐯",
            "fur_leopard": "Léopard 🐆",
            "fur_cow": "Vache 🐮",
            "fur_siamese": "Siamois 🐾",
            "fur_tiger": "Tigre 🐅",
            "fur_dog": "Poil de chien 🐶",
            "dog_source": "Teinte du pelage :",
            "dog_theme": "Fond d'écran",
            "dog_custom": "Couleur",
            "dog_gradient": "Dégradé",
            "color_top": "Haut",
            "color_bottom": "Bas",
            "typing": "Frappe clavier",
            "typing_none": "Off",
            "typing_bounce": "Rebond",
            "typing_rings": "Anneaux",
            "typing_pulse": "Pulse",
            "typing_input_needed": "Accès aux périphériques requis (sudo usermod -aG input $USER)",
            "display": "Écran",
            "display_primary": "Écran principal",
            "follow_cursor": "Suivre le curseur",
            "react_desktop": "Réagir au bureau",
            "react_notifications": "Réagir aux notifications",
            "idle_antics": "Comportements spontanés",
            "sleep_idle": "Dormir si inactif",
            "stay_top": "Toujours au premier plan",
            "reduce_motion": "Réduire les animations",
            "start_login": "Lancer au démarrage",
            "show_tray": "Icône dans la barre des tâches",
            "mochi_petting": "Caresses et cœurs au survol",
            "mochi_bubbles": "Bulles de dialogue (pensées)",
            "interval": "Intervalle :",
            "snacks": "Friandises :",
            "snack_cookie": "🍪 Cookie",
            "snack_coffee": "☕ Café",
            "snack_fish": "🐟 Poisson",
            "snack_donut": "🍩 Donut",
            "snack_strawberry": "🍓 Fraise",
            "snack_tea": "🍵 Thé",
            "accessories": "Accessoires",
            "acc_none": "Aucun ❌",
            "acc_headphones": "Casque 🎧",
            "acc_sunglasses": "Lunettes 😎",
            "acc_glasses": "Rondes 👓",
            "acc_crown": "Couronne 👑",
            "acc_flower": "Fleur 🌸",
            "acc_nightcap": "Bonnet 💤",
            "toys_title": "Jeux & Mini-jeux",
            "toy_laser": "🔴 Laser",
            "toy_yarn": "🧶 Pelote",
            "toy_peekaboo": "🙈 Coucou",
            "toy_celebrate": "🎉 Fête",
            "quit": "Quitter Nala",
            "reset_place": "Réinitialiser la position"
        },
        "en": {
            "title": "Nala",
            "close": "Close preferences",
            "size": "Size",
            "color": "Colour",
            "color_ink": "Ink",
            "color_theme": "Wallpaper",
            "color_custom": "Custom",
            "color_gradient": "Gradient",
            "color_fur": "Fur",
            "fur_calico": "Calico 🐱",
            "fur_tabby": "Tabby 🐯",
            "fur_leopard": "Leopard 🐆",
            "fur_cow": "Spotted / Cow 🐮",
            "fur_siamese": "Siamese 🐾",
            "fur_tiger": "Tiger 🐅",
            "fur_dog": "Dog fur 🐶",
            "dog_source": "Coat tone:",
            "dog_theme": "Wallpaper",
            "dog_custom": "Colour",
            "dog_gradient": "Gradient",
            "color_top": "Top",
            "color_bottom": "Bottom",
            "typing": "Typing",
            "typing_none": "Off",
            "typing_bounce": "Bounce",
            "typing_rings": "Rings",
            "typing_pulse": "Pulse",
            "typing_input_needed": "Needs input access (sudo usermod -aG input $USER)",
            "display": "Display",
            "display_primary": "Primary display",
            "follow_cursor": "Follow the cursor",
            "react_desktop": "React to your desktop",
            "react_notifications": "React to notifications",
            "idle_antics": "Idle antics",
            "sleep_idle": "Sleep when idle",
            "stay_top": "Stay above windows",
            "reduce_motion": "Reduce motion",
            "start_login": "Start at login",
            "show_tray": "Show tray icon",
            "mochi_petting": "Petting & Hearts on hover",
            "mochi_bubbles": "Companion speech bubbles",
            "interval": "Interval:",
            "snacks": "Treats:",
            "snack_cookie": "🍪 Cookie",
            "snack_coffee": "☕ Coffee",
            "snack_fish": "🐟 Fish",
            "snack_donut": "🍩 Donut",
            "snack_strawberry": "🍓 Strawberry",
            "snack_tea": "🍵 Tea",
            "accessories": "Accessories",
            "acc_none": "None ❌",
            "acc_headphones": "Headphones 🎧",
            "acc_sunglasses": "Sunglasses 😎",
            "acc_glasses": "Glasses 👓",
            "acc_crown": "Crown 👑",
            "acc_flower": "Flower 🌸",
            "acc_nightcap": "Nightcap 💤",
            "toys_title": "Mini-games & Fun",
            "toy_laser": "🔴 Laser",
            "toy_yarn": "🧶 Yarn",
            "toy_peekaboo": "🙈 Peek-a-boo",
            "toy_celebrate": "🎉 Party",
            "quit": "Quit Nala",
            "reset_place": "Reset position"
        }
    })

    function t(key) {
        const lang = backend.language === "en" ? "en" : "fr";
        return i18n[lang][key] || key;
    }

    objectName: "settingsWindow"
    title: "Nala"
    width: 600
    height: Math.min(980, Screen.height - 60)
    minimumWidth: 460
    minimumHeight: 520
    flags: Qt.Dialog | Qt.WindowStaysOnTopHint
    visible: false
    color: theme.colors.surface

    onClosing: function (close) {
        close.accepted = false;
        hide();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Draggable Header Bar
        Rectangle {
            id: headerBar
            Layout.fillWidth: true
            implicitHeight: 56
            color: theme.colors.surface

            MouseArea {
                anchors.fill: parent
                onPressed: settings.startSystemMove()
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                spacing: 12

                LabelText {
                    text: t("title")
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                }

                Item {
                    Layout.fillWidth: true
                }

                // Language Selector
                RowLayout {
                    spacing: 4

                    ActionButton {
                        text: "FR"
                        selected: backend.language === "fr"
                        onClicked: backend.configure("language", "fr")
                    }

                    ActionButton {
                        text: "EN"
                        selected: backend.language === "en"
                        onClicked: backend.configure("language", "en")
                    }
                }

                Item {
                    width: 4
                }

                ActionButton {
                    objectName: "closeSettings"
                    text: "×"
                    Accessible.name: t("close")
                    onClicked: settings.hide()
                }
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: theme.colors.outline
            opacity: 0.3
        }

        // Scrollable Body
        ScrollView {
            id: scrollBody
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                width: scrollBody.availableWidth
                spacing: 16

                Item { Layout.preferredHeight: 4 }

                // Level & XP Evolution Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    Layout.rightMargin: 24
                    implicitHeight: xpCol.implicitHeight + 24
                    radius: cardRadius
                    color: Qt.rgba(theme.colors.text.r, theme.colors.text.g, theme.colors.text.b, 0.05)
                    border.color: Qt.rgba(theme.colors.text.r, theme.colors.text.g, theme.colors.text.b, 0.12)
                    border.width: 1

                    ColumnLayout {
                        id: xpCol
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            LabelText {
                                text: "⭐ " + (backend.language === "en" ? "Level " : "Niveau ") + backend.level
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                            }

                            Rectangle {
                                implicitHeight: 22
                                implicitWidth: titleLabel.implicitWidth + 12
                                radius: 11
                                color: Qt.rgba(theme.colors.accent.r, theme.colors.accent.g, theme.colors.accent.b, 0.18)
                                border.color: theme.colors.accent
                                border.width: 1

                                LabelText {
                                    id: titleLabel
                                    anchors.centerIn: parent
                                    text: backend.levelTitle
                                    font.pixelSize: 11
                                    font.weight: Font.Medium
                                    color: theme.colors.accent
                                }
                            }

                            Item { Layout.fillWidth: true }

                            LabelText {
                                text: backend.xpInLevel + " / " + backend.xpForNextLevel + " XP (" + Math.round(backend.levelProgress * 100) + "%)"
                                font.pixelSize: 12
                                color: theme.colors.muted
                            }
                        }

                        // Progress Bar
                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 8
                            radius: 4
                            color: Qt.rgba(theme.colors.text.r, theme.colors.text.g, theme.colors.text.b, 0.12)

                            Rectangle {
                                width: Math.max(0, Math.min(parent.width, parent.width * backend.levelProgress))
                                height: parent.height
                                radius: 4
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: "#ec4899" }
                                    GradientStop { position: 1.0; color: "#a855f7" }
                                }
                                Behavior on width {
                                    NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
                                }
                            }
                        }

                        // Next unlock teaser
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            LabelText {
                                text: (backend.language === "en" ? "Next: " : "Prochain déblocage : ") + backend.nextUnlock
                                font.pixelSize: 12
                                color: theme.colors.muted
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                GridLayout {
                    columns: 2
                    columnSpacing: 20
                    rowSpacing: 10
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    Layout.rightMargin: 24

                    LabelText {
                        text: t("size")
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        NalaSlider {
                            objectName: "sizeSlider"
                            Layout.fillWidth: true
                            from: 0.55
                            to: 2.2
                            stepSize: 0.05
                            value: backend.size
                            onMoved: backend.configure("size", value)
                            Accessible.name: "Mascot size"
                        }

                        LabelText {
                            text: Math.round(backend.size * 100) + "%"
                            color: theme.colors.muted
                            Layout.preferredWidth: 38
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    LabelText {
                        text: t("color")
                    }

                    GridLayout {
                        columns: 5
                        Layout.fillWidth: true
                        rowSpacing: 6
                        columnSpacing: 6

                        ActionButton {
                            objectName: "colorInk"
                            text: t("color_ink")
                            Layout.fillWidth: true
                            selected: backend.colorMode === "ink"
                            onClicked: backend.configure("colorMode", "ink")
                        }

                        ActionButton {
                            objectName: "colorTheme"
                            text: t("color_theme")
                            Layout.fillWidth: true
                            selected: backend.colorMode === "theme"
                            onClicked: backend.configure("colorMode", "theme")
                        }

                        ActionButton {
                            objectName: "colorCustom"
                            text: t("color_custom")
                            Layout.fillWidth: true
                            selected: backend.colorMode === "custom"
                            onClicked: backend.configure("colorMode", "custom")
                        }

                        ActionButton {
                            objectName: "colorGradient"
                            text: t("color_gradient")
                            Layout.fillWidth: true
                            selected: backend.colorMode === "gradient"
                            onClicked: backend.configure("colorMode", "gradient")
                        }

                        ActionButton {
                            objectName: "colorFur"
                            text: t("color_fur")
                            Layout.fillWidth: true
                            selected: backend.colorMode === "fur"
                            onClicked: backend.configure("colorMode", "fur")
                        }
                    }

                    Item {
                        visible: backend.colorMode === "fur"
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 1
                    }

                    ColumnLayout {
                        visible: backend.colorMode === "fur"
                        Layout.fillWidth: true
                        spacing: 8

                        GridLayout {
                            columns: 3
                            Layout.fillWidth: true
                            rowSpacing: 8
                            columnSpacing: 8

                            Repeater {
                                model: [
                                    { id: "calico", key: "fur_calico", colors: ["#f5f2eb", "#e07a3c", "#2b1d16"] },
                                    { id: "tabby", key: "fur_tabby", colors: ["#d4c2b0", "#78604c", "#2b1d16"] },
                                    { id: "leopard", key: "fur_leopard", colors: ["#f0c268", "#ba6820", "#2b1d16"] },
                                    { id: "cow", key: "fur_cow", colors: ["#f5f5f7", "#1e1e24"] },
                                    { id: "siamese", key: "fur_siamese", colors: ["#ede4d8", "#704838", "#2b1d16"] },
                                    { id: "tiger", key: "fur_tiger", colors: ["#f3722c", "#f5ebd7", "#181210"] },
                                    { id: "dog", key: "fur_dog", colors: ["#d8a46e", "#a66d3a", "#543318"] }
                                ]

                                delegate: Rectangle {
                                    required property var modelData
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 38
                                    radius: settings.controlRadius
                                    color: backend.furPattern === modelData.id ? theme.colors.card : theme.colors.surface
                                    border.width: backend.furPattern === modelData.id ? 2 : 1
                                    border.color: backend.furPattern === modelData.id ? theme.colors.accent : theme.colors.outline

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 6
                                        spacing: 8

                                        Row {
                                            spacing: 2
                                            Layout.alignment: Qt.AlignVCenter
                                            Repeater {
                                                model: modelData.colors
                                                delegate: Rectangle {
                                                    required property string modelData
                                                    width: 7
                                                    height: 16
                                                    radius: 3
                                                    color: modelData
                                                }
                                            }
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            text: t(modelData.key)
                                            font.family: theme.fontFamily
                                            font.pixelSize: 12
                                            font.weight: backend.furPattern === modelData.id ? Font.DemiBold : Font.Normal
                                            color: theme.colors.text
                                            elide: Text.ElideRight
                                            verticalAlignment: Text.AlignVCenter
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: backend.configure("furPattern", modelData.id)
                                    }
                                }
                            }
                        }

                        // Dog fur dynamic color source selector
                        ColumnLayout {
                            visible: backend.furPattern === "dog"
                            Layout.fillWidth: true
                            spacing: 8
                            Layout.topMargin: 4

                            LabelText {
                                text: t("dog_source")
                                font.weight: Font.DemiBold
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                ActionButton {
                                    text: t("dog_theme")
                                    Layout.fillWidth: true
                                    selected: backend.dogFurSource === "theme"
                                    onClicked: backend.configure("dogFurSource", "theme")
                                }

                                ActionButton {
                                    text: t("dog_custom")
                                    Layout.fillWidth: true
                                    selected: backend.dogFurSource === "custom"
                                    onClicked: backend.configure("dogFurSource", "custom")
                                }

                                ActionButton {
                                    text: t("dog_gradient")
                                    Layout.fillWidth: true
                                    selected: backend.dogFurSource === "gradient"
                                    onClicked: backend.configure("dogFurSource", "gradient")
                                }
                            }

                            // Sub-panel when dogFurSource is custom: color swatches + hex input
                            ColumnLayout {
                                visible: backend.dogFurSource === "custom"
                                Layout.fillWidth: true
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Layout.fillWidth: true

                                    Repeater {
                                        model: ["#c8965a", "#8c5828", "#e5b882", "#3b82f6", "#10b981", "#ef4444", "#a855f7", "#ffffff"]
                                        delegate: Rectangle {
                                            required property string modelData
                                            width: 24
                                            height: 24
                                            radius: 12
                                            color: modelData
                                            border.width: backend.customColor.toLowerCase() === modelData.toLowerCase() ? 2 : 1
                                            border.color: backend.customColor.toLowerCase() === modelData.toLowerCase() ? theme.colors.text : theme.colors.outline

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: backend.configure("customColor", modelData)
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Rectangle {
                                        width: 28
                                        height: 28
                                        radius: settings.controlRadius / 2
                                        color: backend.customColor
                                        border.width: 1
                                        border.color: theme.colors.outline
                                    }

                                    NalaTextField {
                                        Layout.fillWidth: true
                                        text: backend.customColor
                                        placeholderText: "#RRGGBB"
                                        onTextEdited: {
                                            if (/^#[0-9a-fA-F]{6}$/.test(text)) {
                                                backend.configure("customColor", text);
                                            }
                                        }
                                    }
                                }
                            }

                            // Sub-panel when dogFurSource is gradient: gradient presets + top/bottom hex inputs
                            ColumnLayout {
                                visible: backend.dogFurSource === "gradient"
                                Layout.fillWidth: true
                                spacing: 8

                                RowLayout {
                                    spacing: 8
                                    Layout.fillWidth: true

                                    Repeater {
                                        model: [
                                            { top: "#e0a96d", bottom: "#492810" },
                                            { top: "#ffffff", bottom: "#8d99ae" },
                                            { top: "#ff9a9e", bottom: "#fecfef" },
                                            { top: "#4facfe", bottom: "#00f2fe" },
                                            { top: "#f59e0b", bottom: "#ef4444" },
                                            { top: "#6366f1", bottom: "#a855f7" }
                                        ]
                                        delegate: Rectangle {
                                            required property var modelData
                                            width: 32
                                            height: 24
                                            radius: 6
                                            border.width: (backend.customColor.toLowerCase() === modelData.top.toLowerCase() &&
                                                           backend.gradientColor.toLowerCase() === modelData.bottom.toLowerCase()) ? 2 : 1
                                            border.color: (backend.customColor.toLowerCase() === modelData.top.toLowerCase() &&
                                                           backend.gradientColor.toLowerCase() === modelData.bottom.toLowerCase()) ? theme.colors.text : theme.colors.outline

                                            gradient: Gradient {
                                                GradientStop { position: 0.0; color: modelData.top }
                                                GradientStop { position: 1.0; color: modelData.bottom }
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    backend.configure("customColor", modelData.top);
                                                    backend.configure("gradientColor", modelData.bottom);
                                                }
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6
                                        Rectangle {
                                            width: 22
                                            height: 22
                                            radius: 11
                                            color: backend.customColor
                                            border.width: 1
                                            border.color: theme.colors.outline
                                        }
                                        NalaTextField {
                                            Layout.fillWidth: true
                                            text: backend.customColor
                                            placeholderText: t("color_top")
                                            onTextEdited: {
                                                if (/^#[0-9a-fA-F]{6}$/.test(text)) {
                                                    backend.configure("customColor", text);
                                                }
                                            }
                                        }
                                    }

                                    RowLayout {
                                        Layout.fillWidth: true
                                        spacing: 6
                                        Rectangle {
                                            width: 22
                                            height: 22
                                            radius: 11
                                            color: backend.gradientColor
                                            border.width: 1
                                            border.color: theme.colors.outline
                                        }
                                        NalaTextField {
                                            Layout.fillWidth: true
                                            text: backend.gradientColor
                                            placeholderText: t("color_bottom")
                                            onTextEdited: {
                                                if (/^#[0-9a-fA-F]{6}$/.test(text)) {
                                                    backend.configure("gradientColor", text);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        visible: backend.colorMode === "custom"
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 1
                    }

                    ColumnLayout {
                        visible: backend.colorMode === "custom"
                        Layout.fillWidth: true
                        spacing: 8

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true

                            Repeater {
                                model: ["#a855f7", "#ec4899", "#3b82f6", "#10b981", "#f59e0b", "#ef4444", "#06b6d4", "#e2e8f0"]
                                delegate: Rectangle {
                                    required property string modelData
                                    width: 24
                                    height: 24
                                    radius: 12
                                    color: modelData
                                    border.width: backend.customColor.toLowerCase() === modelData.toLowerCase() ? 2 : 1
                                    border.color: backend.customColor.toLowerCase() === modelData.toLowerCase() ? theme.colors.text : theme.colors.outline

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: backend.configure("customColor", modelData)
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                width: 28
                                height: 28
                                radius: settings.controlRadius / 2
                                color: backend.customColor
                                border.width: 1
                                border.color: theme.colors.outline
                            }

                            NalaTextField {
                                Layout.fillWidth: true
                                text: backend.customColor
                                placeholderText: "#RRGGBB"
                                onTextEdited: {
                                    if (/^#[0-9a-fA-F]{6}$/.test(text)) {
                                        backend.configure("customColor", text);
                                    }
                                }
                            }
                        }
                    }

                    Item {
                        visible: backend.colorMode === "gradient"
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 1
                    }

                    ColumnLayout {
                        visible: backend.colorMode === "gradient"
                        Layout.fillWidth: true
                        spacing: 8

                        RowLayout {
                            spacing: 8
                            Layout.fillWidth: true

                            Repeater {
                                model: [
                                    { top: "#ff007f", bottom: "#7928ca" },
                                    { top: "#00f2fe", bottom: "#4facfe" },
                                    { top: "#10b981", bottom: "#06b6d4" },
                                    { top: "#f59e0b", bottom: "#ef4444" },
                                    { top: "#6366f1", bottom: "#a855f7" },
                                    { top: "#ff9a9e", bottom: "#fecfef" }
                                ]
                                delegate: Rectangle {
                                    required property var modelData
                                    width: 32
                                    height: 24
                                    radius: 6
                                    border.width: (backend.customColor.toLowerCase() === modelData.top.toLowerCase() &&
                                                   backend.gradientColor.toLowerCase() === modelData.bottom.toLowerCase()) ? 2 : 1
                                    border.color: (backend.customColor.toLowerCase() === modelData.top.toLowerCase() &&
                                                   backend.gradientColor.toLowerCase() === modelData.bottom.toLowerCase()) ? theme.colors.text : theme.colors.outline

                                    gradient: Gradient {
                                        GradientStop { position: 0.0; color: modelData.top }
                                        GradientStop { position: 1.0; color: modelData.bottom }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            backend.configure("customColor", modelData.top);
                                            backend.configure("gradientColor", modelData.bottom);
                                        }
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: backend.customColor
                                    border.width: 1
                                    border.color: theme.colors.outline
                                }
                                NalaTextField {
                                    Layout.fillWidth: true
                                    text: backend.customColor
                                    placeholderText: t("color_top")
                                    onTextEdited: {
                                        if (/^#[0-9a-fA-F]{6}$/.test(text)) {
                                            backend.configure("customColor", text);
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                Rectangle {
                                    width: 22
                                    height: 22
                                    radius: 11
                                    color: backend.gradientColor
                                    border.width: 1
                                    border.color: theme.colors.outline
                                }
                                NalaTextField {
                                    Layout.fillWidth: true
                                    text: backend.gradientColor
                                    placeholderText: t("color_bottom")
                                    onTextEdited: {
                                        if (/^#[0-9a-fA-F]{6}$/.test(text)) {
                                            backend.configure("gradientColor", text);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    LabelText {
                        text: t("typing")
                    }

                    GridLayout {
                        columns: 4
                        Layout.fillWidth: true
                        rowSpacing: 6
                        columnSpacing: 6

                        ActionButton {
                            objectName: "typingNone"
                            text: t("typing_none")
                            Layout.fillWidth: true
                            selected: backend.keyboardReaction === "none"
                            onClicked: backend.configure("keyboardReaction", "none")
                        }

                        ActionButton {
                            objectName: "typingBounce"
                            text: t("typing_bounce")
                            Layout.fillWidth: true
                            selected: backend.keyboardReaction === "bounce"
                            onClicked: backend.configure("keyboardReaction", "bounce")
                        }

                        ActionButton {
                            objectName: "typingRings"
                            text: t("typing_rings")
                            Layout.fillWidth: true
                            selected: backend.keyboardReaction === "rings"
                            onClicked: backend.configure("keyboardReaction", "rings")
                        }

                        ActionButton {
                            objectName: "typingPulse"
                            text: t("typing_pulse")
                            Layout.fillWidth: true
                            selected: backend.keyboardReaction === "pulse"
                            onClicked: backend.configure("keyboardReaction", "pulse")
                        }
                    }

                    Item {
                        visible: !backend.keyboardAvailable && backend.keyboardReaction !== "none"
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 1
                    }

                    LabelText {
                        visible: !backend.keyboardAvailable && backend.keyboardReaction !== "none"
                        text: t("typing_input_needed")
                        font.pixelSize: 11
                        color: theme.colors.muted
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    LabelText {
                        text: t("display")
                    }

                    ComboBox {
                        id: displayPicker

                        objectName: "displayPicker"
                        Layout.fillWidth: true
                        implicitHeight: 32
                        leftPadding: 12
                        rightPadding: 30
                        model: [t("display_primary")].concat(backend.screens)
                        currentIndex: backend.monitor.length ? Math.max(0, backend.screens.indexOf(backend.monitor) + 1) : 0
                        onActivated: backend.configure("monitor", currentIndex === 0 ? "" : backend.screens[currentIndex - 1])

                        contentItem: LabelText {
                            text: displayPicker.displayText
                            elide: Text.ElideRight
                        }

                        indicator: LabelText {
                            text: "⌄"
                            x: displayPicker.width - 24
                            y: (displayPicker.height - height) / 2
                        }

                        background: Rectangle {
                            radius: settings.controlRadius
                            color: displayPicker.hovered ? theme.colors.hover : theme.colors.card
                            border.width: displayPicker.activeFocus ? 1 : 0
                            border.color: theme.colors.accent
                        }

                        delegate: ItemDelegate {
                            id: displayOption

                            required property int index
                            required property string modelData

                            width: displayPicker.width - 12
                            implicitHeight: 34
                            highlighted: displayPicker.highlightedIndex === index

                            contentItem: LabelText {
                                text: displayOption.modelData
                            }

                            background: Rectangle {
                                radius: settings.controlRadius
                                color: displayOption.highlighted || displayOption.hovered ? theme.colors.hover : "transparent"
                            }
                        }

                        popup: Popup {
                            y: displayPicker.height + 4
                            width: displayPicker.width
                            padding: 6
                            implicitHeight: contentItem.implicitHeight + 12

                            background: Rectangle {
                                radius: settings.cardRadius
                                color: theme.colors.card
                                border.width: 1
                                border.color: theme.colors.outline
                            }

                            contentItem: ListView {
                                clip: true
                                implicitHeight: contentHeight
                                model: displayPicker.popup.visible ? displayPicker.delegateModel : null
                                currentIndex: displayPicker.highlightedIndex
                            }
                        }
                    }

                    LabelText {
                        text: t("accessories")
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 6

                        Repeater {
                            model: [
                                { key: "none", label: t("acc_none") },
                                { key: "headphones", label: t("acc_headphones") },
                                { key: "sunglasses", label: t("acc_sunglasses") },
                                { key: "glasses", label: t("acc_glasses") },
                                { key: "crown", label: t("acc_crown") },
                                { key: "flower", label: t("acc_flower") },
                                { key: "nightcap", label: t("acc_nightcap") }
                            ]

                            delegate: ActionButton {
                                required property var modelData
                                text: modelData.label
                                selected: backend.accessory === modelData.key
                                onClicked: backend.configure("accessory", modelData.key)
                            }
                        }
                    }

                    LabelText {
                        text: t("toys_title")
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        ActionButton {
                            text: t("toy_laser")
                            selected: backend.laserActive
                            Layout.fillWidth: true
                            onClicked: backend.toggleLaser()
                        }
                        ActionButton {
                            text: t("toy_yarn")
                            selected: backend.yarnActive
                            Layout.fillWidth: true
                            onClicked: backend.toggleYarn()
                        }
                        ActionButton {
                            text: t("toy_peekaboo")
                            selected: backend.peekabooActive
                            Layout.fillWidth: true
                            onClicked: backend.triggerPeekaboo()
                        }
                        ActionButton {
                            text: t("toy_celebrate")
                            Layout.fillWidth: true
                            onClicked: backend.celebrate()
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    Layout.rightMargin: 24
                    spacing: 4

                    NalaSwitch {
                        objectName: "followToggle"
                        text: t("follow_cursor")
                        checked: backend.followCursor
                        onToggled: backend.configure("followCursor", checked)
                    }

                    NalaSwitch {
                        objectName: "desktopToggle"
                        text: t("react_desktop")
                        checked: backend.reactToDesktop
                        onToggled: backend.configure("reactToDesktop", checked)
                    }

                    NalaSwitch {
                        objectName: "notificationsToggle"
                        text: t("react_notifications")
                        enabled: backend.reactToDesktop
                        checked: backend.reactToNotifications
                        onToggled: backend.configure("reactToNotifications", checked)
                    }

                    NalaSwitch {
                        objectName: "anticsToggle"
                        text: t("idle_antics")
                        checked: backend.idleAntics
                        onToggled: backend.configure("idleAntics", checked)
                    }

                    NalaSwitch {
                        objectName: "sleepToggle"
                        text: t("sleep_idle")
                        checked: backend.sleepWhenIdle
                        onToggled: backend.configure("sleepWhenIdle", checked)
                    }

                    NalaSwitch {
                        objectName: "topToggle"
                        text: t("stay_top")
                        checked: backend.stayOnTop
                        onToggled: backend.configure("stayOnTop", checked)
                    }

                    NalaSwitch {
                        objectName: "motionToggle"
                        text: t("reduce_motion")
                        checked: backend.reducedMotion
                        onToggled: backend.configure("reducedMotion", checked)
                    }

                    NalaSwitch {
                        objectName: "loginToggle"
                        text: t("start_login")
                        checked: backend.startAtLogin
                        onToggled: {
                            backend.setStartAtLogin(checked);
                            checked = Qt.binding(function () {
                                return backend.startAtLogin;
                            });
                        }
                    }

                    NalaSwitch {
                        objectName: "trayToggle"
                        text: t("show_tray")
                        checked: backend.showTrayIcon
                        onToggled: backend.configure("showTrayIcon", checked)
                    }

                    NalaSwitch {
                        objectName: "mochiPettingToggle"
                        text: t("mochi_petting")
                        checked: backend.mochiPetting
                        onToggled: backend.configure("mochiPetting", checked)
                    }

                    NalaSwitch {
                        objectName: "mochiBubblesToggle"
                        text: t("mochi_bubbles")
                        checked: backend.mochiBubbles
                        onToggled: backend.configure("mochiBubbles", checked)
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        visible: backend.mochiBubbles
                        spacing: 12
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4

                        LabelText {
                            text: t("interval")
                            Layout.preferredWidth: 65
                        }

                        NalaSlider {
                            objectName: "bubbleIntervalSlider"
                            Layout.fillWidth: true
                            from: 1
                            to: 10
                            stepSize: 1
                            value: Math.max(1, Math.min(10, Math.round(backend.mochiInterval / 60)))
                            onMoved: backend.configure("mochiInterval", value * 60)
                            Accessible.name: t("interval")
                        }

                        LabelText {
                            text: Math.max(1, Math.min(10, Math.round(backend.mochiInterval / 60))) + " min"
                            color: theme.colors.muted
                            Layout.preferredWidth: 44
                            horizontalAlignment: Text.AlignRight
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4

                        LabelText {
                            text: t("snacks")
                        }

                        GridLayout {
                            columns: 3
                            Layout.fillWidth: true
                            rowSpacing: 6
                            columnSpacing: 6

                            ActionButton {
                                Layout.fillWidth: true
                                text: t("snack_cookie")
                                onClicked: backend.feed("cookie")
                            }

                            ActionButton {
                                Layout.fillWidth: true
                                text: t("snack_coffee")
                                onClicked: backend.feed("coffee")
                            }

                            ActionButton {
                                Layout.fillWidth: true
                                text: t("snack_fish")
                                onClicked: backend.feed("fish")
                            }

                            ActionButton {
                                Layout.fillWidth: true
                                text: t("snack_donut")
                                onClicked: backend.feed("donut")
                            }

                            ActionButton {
                                Layout.fillWidth: true
                                text: t("snack_strawberry")
                                onClicked: backend.feed("strawberry")
                            }

                            ActionButton {
                                Layout.fillWidth: true
                                text: t("snack_tea")
                                onClicked: backend.feed("tea")
                            }
                        }
                    }
                }

                LabelText {
                    Layout.fillWidth: true
                    Layout.leftMargin: 24
                    Layout.rightMargin: 24
                    visible: backend.feedback.length > 0
                    text: backend.feedback
                    color: theme.colors.error
                    wrapMode: Text.Wrap
                }

                Item { Layout.preferredHeight: 8 }
            }
        }

        // Fixed Footer
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 60
            color: theme.colors.surface

            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 1
                color: theme.colors.outline
                opacity: 0.3
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24

                ActionButton {
                    text: t("quit")
                    onClicked: backend.quit()
                }

                Item {
                    Layout.fillWidth: true
                }

                ActionButton {
                    objectName: "resetPlace"
                    text: t("reset_place")
                    onClicked: backend.resetPlace()
                }
            }
        }
    }

    component LabelText: Text {
        color: theme.colors.text
        font.pixelSize: 13
        font.family: theme.fontFamily
        verticalAlignment: Text.AlignVCenter
    }

    component ActionButton: Button {
        id: btn

        property bool selected: false

        implicitHeight: 32
        implicitWidth: Math.max(32, contentItem.implicitWidth + 16)
        padding: 6

        background: Rectangle {
            radius: settings.controlRadius
            color: btn.selected ? theme.colors.accent : btn.down ? theme.colors.pressed : btn.hovered ? theme.colors.hover : theme.colors.card
            border.width: btn.activeFocus ? 1 : 0
            border.color: theme.colors.accent

            Behavior on color {
                ColorAnimation {
                    duration: settings.motion
                }
            }
        }

        contentItem: LabelText {
            text: btn.text
            color: btn.selected ? theme.colors.onAccent : theme.colors.text
            opacity: btn.enabled ? 1 : 0.3
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
    }

    component NalaSlider: Slider {
        id: control

        implicitHeight: 30

        background: Rectangle {
            x: control.leftPadding
            y: control.topPadding + control.availableHeight / 2 - 2
            width: control.availableWidth
            height: 4
            radius: 2
            color: theme.colors.outline

            Rectangle {
                width: control.visualPosition * parent.width
                height: 4
                radius: 2
                color: theme.colors.accent
            }
        }

        handle: Rectangle {
            x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: control.topPadding + control.availableHeight / 2 - height / 2
            width: 16
            height: 16
            radius: 8
            color: control.pressed ? theme.colors.text : theme.colors.accent
            border.width: control.activeFocus ? 2 : 0
            border.color: theme.colors.text

            Behavior on color {
                ColorAnimation {
                    duration: settings.motion
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    component NalaSwitch: Switch {
        id: control

        opacity: enabled ? 1 : 0.4
        Layout.fillWidth: true
        implicitHeight: 34
        padding: 0
        leftPadding: 0
        rightPadding: 48

        contentItem: LabelText {
            text: control.text
        }

        indicator: Rectangle {
            x: control.width - width
            y: (control.height - height) / 2
            width: 36
            height: 22
            radius: 11
            color: control.checked ? theme.colors.accent : theme.colors.outline
            border.width: control.activeFocus ? 1 : 0
            border.color: theme.colors.text

            Rectangle {
                x: control.checked ? 17 : 3
                y: 3
                width: 16
                height: 16
                radius: 8
                color: control.checked ? theme.colors.onAccent : theme.colors.muted

                Behavior on x {
                    NumberAnimation {
                        duration: settings.motion
                        easing.type: Easing.OutCubic
                    }
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: settings.motion
                }
            }
        }
    }

    component NalaTextField: TextField {
        id: field
        color: theme.colors.text
        font.pixelSize: 13
        font.family: theme.fontFamily
        leftPadding: 10
        rightPadding: 10
        topPadding: 5
        bottomPadding: 5
        selectByMouse: true
        background: Rectangle {
            radius: settings.controlRadius
            color: field.activeFocus ? theme.colors.surface : theme.colors.card
            border.width: field.activeFocus ? 1 : 0
            border.color: theme.colors.accent
        }
    }
}
