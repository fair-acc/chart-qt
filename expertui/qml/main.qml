import QtQuick.Controls 2.3
import ChartQt 1.0
import ExpertUi 1.0
import QtQuick 2.0

ApplicationWindow {
    visible: true
    width: 1980
    height: 450

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            Action {
                text: "foo"
            }
        }
    }

    header: ToolBar {
        ToolButton {
            text: chart.paused ? "Resume" : "Pause"
            onClicked: chart.paused = !chart.paused
        }
    }

    SinDataSet
    {
        id: sinDataSet
    }

    ChartLayout {
        anchors.top: parent.top
        anchors.left: fields.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        orientation: Qt.Vertical

        ChartItem {
            id: chart
            clip: true

            DefaultChartInputHandler {}

            XYPlot {
                id: xy
                xAxis: bottomAxis
                yAxis: leftAxis
                dataSet: sinDataSet
            }

            Axis {
                id: bottomAxis
                position: Axis.Bottom
                max: 10
            }

            Axis {
                id: leftAxis
                position: Axis.Left
                min: -1
                max: 1
            }

            Axis {
                id: leftAxis1
                position: Axis.Left
                min: 0
                max: 10
            }

            MouseArea {
                id: topHandle
                width: 20
                height: 10
                anchors.right: parent.right
                y: parent.height * 0.2 - height / 2
                cursorShape: Qt.SizeVerCursor

                property real pressY
                onPressed: (mouse) => pressY = mouse.y
                onPositionChanged: (mouse) => {
                    y += mouse.y - pressY
                }
            }

            MouseArea {
                id: bottomHandle
                width: 20
                anchors.right: parent.right
                cursorShape: Qt.SizeVerCursor
                y: parent.height * 0.8 - height / 2
                height: 10

                property real pressY
                onPressed: (mouse) => pressY = mouse.y
                onPositionChanged: (mouse) => {
                    y += mouse.y - pressY
                }
            }

            Rectangle {
                anchors.right: parent.right
                width: 20
                anchors.top: topHandle.verticalCenter
                anchors.bottom: bottomHandle.verticalCenter


                gradient: Gradient {
                    GradientStop { position: 0.0; color: "green" }
                    GradientStop { position: 1.0; color: "red" }
                }
            }
        }
        ChartItem {
            id: waterfall

            Component.onCompleted: waterfall.addAxis(bottomAxis)
                Axis {
                position: Axis.Right
                min: 0
                max: 10
            }

            WaterfallPlot {
                id: wf

                gradientStart: 1 - (bottomHandle.y + bottomHandle.height / 2) / chart.height
                gradientStop: 1 - (topHandle.y + topHandle.height / 2) / chart.height

                xAxis: bottomAxis
                dataSet: sinDataSet
            }
        }
    }

    GroupBox {
        id: devices
        title: qsTr("Devices")
        anchors {
            left: parent.left
            top: parent.top
            right: parent.horizontalCenter
        }
        height: 100

        ListView {
            id: devicesList
            anchors.fill: parent
            clip: true
            model: DevicesModel { id: devicesModel }

            ScrollBar.vertical: ScrollBar {
                active: true
                // keep the scrollbar visible after the user interacted with it
                onActiveChanged: active = true
            }

            highlight: Rectangle {
                color: "#00a0ff"
            }

            delegate: MouseArea {
                width: parent.width
                height: 20
                property string name: model.name
                property string address: model.address

                Row {
                    height: parent.height
                    TextInput {
                        id: deviceName
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.name
                        enabled: false
                    }
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: ": "
                    }
                    TextInput {
                        id: deviceAddress
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.address
                        enabled: false
                        width: Math.max(1, contentWidth)

                        onAccepted: {
                            devicesModel.addNewDevice(text)
                            text = ""
                            devicesList.currentIndex = devicesList.count - 2
                        }
                    }
                }

                onClicked: {
                    if (index == devicesList.count - 1) {
                        deviceAddress.enabled = true
                        deviceAddress.forceActiveFocus()
                    } else {
                        devicesList.currentIndex = index
                    }
                }
            }
        }
    }

    TabBar {
        id: tabBar
        anchors {
            left: parent.left
            top: devices.bottom
            topMargin: 10
        }

        TabButton {
            text: qsTr("%1: Acquisition").arg(devicesList.currentItem.name)
            width: implicitWidth
        }
        TabButton {
            text: qsTr("%1: Something").arg(devicesList.currentItem.name)
            width: implicitWidth
        }
    }

    Pane {
        id: fields
        visible: tabBar.currentIndex == 0

        anchors {
            left: parent.left
            top: tabBar.bottom
            bottom: parent.bottom
            right: parent.horizontalCenter
        }

        ListView {
            id: somelist
            anchors {
                left: parent.left
                top: parent.top
                bottom: getButton.top
                right: parent.right
            }

            model: NetworkModel {
                id: modelList
                address: devicesList.currentItem.address
            }

            delegate: Item {
                height: 20
                width: parent ? parent.width : 0
                Text {
                    text: model.name
                }
                TextField {
                    anchors.right: parent.right
                    text: model.value || ""
                    color: text == model.value ? "black" : "red"
                    onAccepted: {
                        modelList.requestSetValue(index, text)
                    }
                }
            }
        }

        Button {
            id: getButton
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            text: qsTr("Get")
            onClicked: modelList.getValues()
        }
    }
}
