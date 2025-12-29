import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: root
    x: photoBackend.get_APP_POSITION_X()
    y: photoBackend.get_APP_POSITION_Y()
    visible: true
    width: photoBackend.get_APP_WIDTH()
    height: photoBackend.get_APP_HEIGTH()
    title: "Pap.er"

    color: "#1a1a1a"


    // 添加方法供 C++ 调用
    function openSettings() {
        console.log("Opening settings from tray")
        settingsWindowLoader.active = true
    }

    // 窗口关闭事件 - 最小化到托盘而不是退出
    onClosing: function(close) {
        close.accepted = false  // 阻止窗口关闭
        root.hide()  // 隐藏窗口
        console.log("Window minimized to tray")
    }


    // 组件加载完成后启动自动更换壁纸
    Component.onCompleted: {
        // 从设置中读取更新间隔并启动定时器
        var intervalMinutes = settingsBackend.getUpdateIntervalMinutes()
        if (intervalMinutes > 0) {
            photoBackend.startAutoWallpaperChange(intervalMinutes)
            console.log("Auto wallpaper change started with interval:", intervalMinutes, "minutes")
        }
    }

    onActiveChanged: {
        if(!active){
            // root.visible=false
            root.close()
        }
        else{
            // systemTray.hide()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            //root.visible=false
            root.close()

        }
    }

    // 设置窗口组件
    Loader {
        id: settingsWindowLoader
        source: "Settings.qml"
        active: false

        onLoaded: {
            item.visible = true
            root.visible = false

            // 监听设置窗口关闭
            item.windowClosed.connect(function() {
                root.visible = true
                settingsWindowLoader.active = false
            })

            // 监听设置保存成功
            item.settingsSaved.connect(function(newFolderPath) {
                if (newFolderPath !== "") {
                    console.log("Updating wallpaper folder to:", newFolderPath)
                    photoBackend.set_m_photoDirectory_base(newFolderPath)

                    // 根据当前标签页重新加载
                    photoBackend.switchTab(tabBar.currentIndex)
                }

                // 更新自动更换壁纸的时间间隔
                var intervalMinutes = settingsBackend.getUpdateIntervalMinutes()
                if (intervalMinutes > 0) {
                    photoBackend.startAutoWallpaperChange(intervalMinutes)
                    console.log("Auto wallpaper interval updated to:", intervalMinutes, "minutes")
                } else {
                    photoBackend.stopAutoWallpaperChange()
                    console.log("Auto wallpaper change stopped")
                }
            })
        }
    }

    header: Rectangle {
        width: parent.width
        height: 60
        color: "#2a2a2a"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            Text {
                text: "Pap.er"
                font.pixelSize: 22
                font.bold: true
                color: "#666666"
                Layout.alignment: Qt.AlignLeft
            }

            Item { Layout.fillWidth: true }


            //test 最小化按钮
            // Button {
            //                Layout.preferredWidth: 36
            //                Layout.preferredHeight: 36
            //                background: Rectangle {
            //                    color: "transparent"
            //                    radius: 18
            //                }
            //                contentItem: Text {
            //                    text: "−"
            //                    font.pixelSize: 28
            //                    color: "#888888"
            //                    horizontalAlignment: Text.AlignHCenter
            //                    verticalAlignment: Text.AlignVCenter
            //                }
            //                onClicked: {
            //                    console.log("Minimize to tray clicked")
            //                    root.hide()
            //                }
            //            }




            // 设置按钮
            Button {
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                background: Rectangle {
                    color: "transparent"
                    radius: 18
                }
                contentItem: Text {
                    text: "⚙"
                    font.pixelSize: 24
                    color: "#888888"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    console.log("Settings clicked")
                    settingsWindowLoader.active = true
                }
            }
        }
    }

    // 标签栏
    Rectangle {
        id: tabBar
        width: parent.width
        height: 50
        color: "#2a2a2a"
        anchors.top: parent.top

        Row {
            anchors.centerIn: parent
            spacing: 60

            Repeater {
                model: ListModel {
                    ListElement { name: "最新"; index: 0 }
                    ListElement { name: "最热"; index: 1 }
                    ListElement { name: "历史"; index: 2 }
                }

                delegate: Item {
                    width: tabText.width
                    height: 50

                    Text {
                        id: tabText
                        text: model.name
                        font.pixelSize: 16
                        color: tabBar.currentIndex === model.index ? "#ffffff" : "#666666"
                        anchors.centerIn: parent
                    }

                    Rectangle {
                        width: parent.width
                        height: 3
                        color: "#4a9eff"
                        anchors.bottom: parent.bottom
                        visible: tabBar.currentIndex === model.index
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            tabBar.currentIndex = model.index
                            photoBackend.switchTab(model.index)
                        }
                    }
                }
            }
        }

        property int currentIndex: 1
    }

    // 照片网格
    ScrollView {
        anchors.top: tabBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        clip: true

        // 隐藏滚动条
        ScrollBar.vertical.policy: ScrollBar.AlwaysOff
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        GridView {
            id: photoGrid
            anchors.fill: parent
            cellWidth: width
            cellHeight: 220
            model: photoModel


            onContentYChanged: {
                if (contentY + height >= contentHeight - 50) {
                    photoBackend.loadMorePhotos()
                }
            }


            delegate: Item {
                width: photoGrid.cellWidth
                height: photoGrid.cellHeight

                Rectangle {
                    anchors.fill: parent
                    color: "#2a2a2a"

                    Image {
                        anchors.fill: parent
                        source: model.imageUrl
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        clip: true

                        Rectangle {
                            width: 200
                            height: 40
                            color: "#80000000"
                            radius: 4
                            visible: model.imageUrl === ""
                            anchors.centerIn: parent

                            Text {
                                text: "图片占位符 " + (index + 1)
                                color: "#ffffff"
                                anchors.centerIn: parent
                                font.pixelSize: 14
                            }
                        }
                    }

                    Button {
                        z: 11 // 要比MouseArea的z大，不然会被遮住，不能点击
                        id: settingBtn
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 10

                        visible: false

                        onClicked: {
                            console.log("鼠标进入按钮")
                            console.log("Button clicked:", index)
                            photoBackend.onPhotoClicked(index)
                            photoBackend.setWallPaper(model.imageUrl)
                            photoBackend.copyImage(model.imageUrl)
                        }

                        // 自定义背景（圆角 + 半透明黑色）
                        background: Rectangle {
                            radius: height / 2          // 圆角（胶囊形）
                            color: "#80000000"          // 半透明黑色（80=50%透明）
                        }

                        // 自定义文字（白色）
                        contentItem: Text {
                            text: "设置"
                            color: "white"
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        // 可选：手动控制按钮大小
                        width: 40
                        height: 24
                    }

                    MouseArea {
                        z: 10 // 要比Button的z小，不然会遮住，导致不能点击
                        anchors.fill: parent
                        hoverEnabled: true

                        onEntered: {
                            //console.log("鼠标进入")
                            settingBtn.visible = true
                        }

                        onExited: {
                            //console.log("鼠标离开")
                            settingBtn.visible = false
                        }
                    }
                }
            }
        }
    }

    // 左下角刷新按钮 - 固定在整个列表左下角
    Rectangle {
        width: 50
        height: 50
        color: "#cc000000"
        radius: 25
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 20
        anchors.bottomMargin: 20

        Text {
            text: "↻"
            font.pixelSize: 28
            color: "#ffffff"
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("Refresh clicked for tab:", tabBar.currentIndex)
                photoBackend.switchTab(tabBar.currentIndex)
            }
        }
    }
}
