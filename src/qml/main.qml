import QtQuick.Controls 2.3
import ChartQt 1.0
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

    ChartItem {
        id: chart
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.verticalCenter

        XYPlot { id: xy }

        Component.onCompleted: {
            chart.addPlot(xy)
        }

        MouseArea {
            id: topHandle
            width: parent.width
            height: 10
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
            width: parent.width
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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: chart.bottom
        anchors.bottom: parent.bottom

        WaterfallPlot {
            id: wf

            gradientStart: 1 - (bottomHandle.y + bottomHandle.height / 2) / chart.height
            gradientStop: 1 - (topHandle.y + topHandle.height / 2) / chart.height
        }

        Component.onCompleted: {
            waterfall.addPlot(wf)
        }


    }

}