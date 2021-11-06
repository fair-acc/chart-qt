import QtQuick.Controls 2.3
import ChartQt 1.0

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
        anchors.fill: parent
    }
}
