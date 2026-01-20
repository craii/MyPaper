import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: settingsWindow
    x: photoBackend.get_APP_POSITION_X()
    y: photoBackend.get_APP_POSITION_Y()
    visible: false
    width: photoBackend.get_APP_WIDTH()
    height: photoBackend.get_APP_HEIGTH()
    title: "设置 - Pap.er"

    color: "#1a1a1a"

    // 自定义信号 - 修改为传递服务器配置参数
    signal windowClosed()
    signal settingsSaved(string newFolderPath, string serverUrl, string token)

    flags: Qt.Window | Qt.FramelessWindowHint

    onActiveChanged: {
        if(!active){
            root.visible=false
            settingsWindow.visible=false
        }
    }

    onVisibleChanged: {
        if (visible) {
            console.log("Settings window opened")
            settingsBackend.loadSettings()
        } else {
            console.log("Settings window closed")
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            // 点击空白处不关闭
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#1a1a1a"

        // 头部
        Rectangle {
            id: header
            width: parent.width
            height: 60
            color: "#2a2a2a"
            anchors.top: parent.top

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20

                Text {
                    text: "设置"
                    font.pixelSize: 22
                    font.bold: true
                    color: "#ffffff"
                    Layout.alignment: Qt.AlignLeft
                }

                Item { Layout.fillWidth: true }

                Button {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    background: Rectangle {
                        color: "transparent"
                        radius: 18
                    }
                    contentItem: Text {
                        text: "✕"
                        font.pixelSize: 24
                        color: "#888888"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        settingsWindow.close()
                        windowClosed()
                    }
                }
            }
        }

        // 设置内容区域
        ScrollView {
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttonRow.top
            anchors.margins: 20
            clip: true

            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                width: parent.parent.width - 40
                spacing: 25

                // 壁纸文件夹
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "壁纸文件夹"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#ffffff"
                    }

                    Text {
                        text: "设置存放壁纸的根目录,将自动创建 latest、hotest、history 子文件夹"
                        font.pixelSize: 12
                        color: "#888888"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 40
                        color: "#2a2a2a"
                        radius: 4
                        border.color: wallpaperFolderInput.activeFocus ? "#4a9eff" : "#3a3a3a"
                        border.width: 1

                        TextInput {
                            id: wallpaperFolderInput
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            verticalAlignment: TextInput.AlignVCenter
                            font.pixelSize: 14
                            color: "#ffffff"
                            selectByMouse: true
                            clip: true

                            Text {
                                text: "例如: D:/wallpapers 或 ~/Pictures/wallpapers"
                                font.pixelSize: 14
                                color: "#666666"
                                visible: parent.text === ""
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }

                    Text {
                        id: folderValidationText
                        text: ""
                        font.pixelSize: 12
                        color: "#ff4444"
                        visible: text !== ""
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }
                }

                // 分隔线
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#3a3a3a"
                }

                // 自动更新间隔
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "自动更新壁纸间隔"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#ffffff"
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            text: "设置自动更换壁纸的时间间隔"
                            font.pixelSize: 12
                            color: "#888888"
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "立即更换"
                            Layout.preferredHeight: 30

                            background: Rectangle {
                                color: parent.pressed ? "#3a7acc" : "#4a9eff"
                                radius: 4
                            }

                            contentItem: Text {
                                text: parent.text
                                font.pixelSize: 12
                                color: "#ffffff"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                console.log("Manual wallpaper change triggered")
                                photoBackend.setRandomWallpaperFromLatest()
                            }
                        }
                    }

                    ComboBox {
                        id: updateIntervalCombo
                        Layout.fillWidth: true
                        model: ["30分钟", "1小时", "1天"]

                        background: Rectangle {
                            color: "#2a2a2a"
                            radius: 4
                            border.color: updateIntervalCombo.pressed ? "#4a9eff" : "#3a3a3a"
                            border.width: 1
                        }

                        contentItem: Text {
                            leftPadding: 10
                            text: updateIntervalCombo.displayText
                            font.pixelSize: 14
                            color: "#ffffff"
                            verticalAlignment: Text.AlignVCenter
                        }

                        popup: Popup {
                            y: updateIntervalCombo.height
                            width: updateIntervalCombo.width
                            padding: 0

                            contentItem: ListView {
                                clip: true
                                implicitHeight: contentHeight
                                model: updateIntervalCombo.popup.visible ? updateIntervalCombo.delegateModel : null
                                currentIndex: updateIntervalCombo.highlightedIndex

                                ScrollIndicator.vertical: ScrollIndicator { }
                            }

                            background: Rectangle {
                                color: "#2a2a2a"
                                border.color: "#3a3a3a"
                                radius: 4
                            }
                        }

                        delegate: ItemDelegate {
                            width: updateIntervalCombo.width

                            contentItem: Text {
                                text: modelData
                                color: "#ffffff"
                                font.pixelSize: 14
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }

                            background: Rectangle {
                                color: highlighted ? "#4a9eff" : "#2a2a2a"
                            }
                        }
                    }
                }

                // 分隔线
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#3a3a3a"
                }

                // 服务器配置
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "服务器配置"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#ffffff"
                    }

                    Text {
                        text: "配置壁纸服务器地址和访问令牌, 留空则读取本地hotest文件夹中的图片"
                        font.pixelSize: 12
                        color: "#888888"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // 服务器地址
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        Text {
                            text: "服务器地址"
                            font.pixelSize: 14
                            color: "#cccccc"
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: "#2a2a2a"
                            radius: 4
                            border.color: serverUrlInput.activeFocus ? "#4a9eff" : "#3a3a3a"
                            border.width: 1

                            TextInput {
                                id: serverUrlInput
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                verticalAlignment: TextInput.AlignVCenter
                                font.pixelSize: 14
                                color: "#ffffff"
                                selectByMouse: true
                                clip: true

                                Text {
                                    text: "例如: https://api.example.com"
                                    font.pixelSize: 14
                                    color: "#666666"
                                    visible: parent.text === ""
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }
                    }

                    // Token
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5

                        Text {
                            text: "访问令牌 (Token)"
                            font.pixelSize: 14
                            color: "#cccccc"
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            color: "#2a2a2a"
                            radius: 4
                            border.color: tokenInput.activeFocus ? "#4a9eff" : "#3a3a3a"
                            border.width: 1

                            TextInput {
                                id: tokenInput
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                verticalAlignment: TextInput.AlignVCenter
                                font.pixelSize: 14
                                color: "#ffffff"
                                selectByMouse: true
                                echoMode: TextInput.Password
                                clip: true
                            }
                        }
                    }
                    Text {
                        text: "项目地址"
                        font.pixelSize: 14
                        color: "#ffffff"
                        font.bold: true
                    }

                    Text {
                        id: serverValidationText
                        text: "<a href=\"https://github.com/craii/MyPaper\">https://github.com/craii/MyPaper</a>"
                        font.pixelSize: 12
                        color: "#ffffff"
                        visible: text !== ""
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        textFormat: Text.RichText
                        linkColor: "#ffffff"
                        onLinkActivated: {
                            Qt.openUrlExternally(link)  // 使用默认浏览器打开链接
                        }

                        // 可选：鼠标悬停时显示手型光标
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            acceptedButtons: Qt.NoButton  // 不拦截点击，让 Text 的 onLinkActivated 处理
                        }
                    }


                }
            }
        }

        // 底部按钮
        Rectangle {
            id: buttonRow
            width: parent.width
            height: 70
            color: "#2a2a2a"
            anchors.bottom: parent.bottom

            RowLayout {
                anchors.centerIn: parent
                spacing: 15

                Button {
                    text: "取消"
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        color: parent.pressed ? "#3a3a3a" : "#2a2a2a"
                        border.color: "#4a4a4a"
                        border.width: 1
                        radius: 4
                    }

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        settingsWindow.close()
                        windowClosed()
                    }
                }

                Button {
                    text: "保存"
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 40

                    background: Rectangle {
                        color: parent.pressed ? "#3a7acc" : "#4a9eff"
                        radius: 4
                    }

                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        color: "#ffffff"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        saveSettings()
                    }
                }
            }
        }
    }

    // 保存设置 - 修改为返回服务器配置
    function saveSettings() {
        folderValidationText.text = ""
        serverValidationText.text = ""

        var folderPath = wallpaperFolderInput.text.trim()
        var serverUrl = serverUrlInput.text.trim()
        var token = tokenInput.text.trim()
        var interval = updateIntervalCombo.currentIndex

        var result = settingsBackend.validateAndSave(
            folderPath,
            serverUrl,
            token,
            interval
        )

        if (result.success) {
            console.log("Settings saved successfully")

            // 发射信号,传递服务器配置参数
            settingsSaved(folderPath, serverUrl, token)

            settingsWindow.close()
            windowClosed()
        } else {
            if (result.folderError) {
                folderValidationText.text = result.folderError
            }
            if (result.serverError) {
                serverValidationText.text = result.serverError
            }
        }
    }

    // 加载设置到界面
    Connections {
        target: settingsBackend
        function onSettingsLoaded(settings) {
            wallpaperFolderInput.text = settings.wallpaperFolder || ""
            serverUrlInput.text = settings.serverUrl || ""
            tokenInput.text = settings.token || ""
            updateIntervalCombo.currentIndex = settings.updateInterval || 0
        }
    }
}
