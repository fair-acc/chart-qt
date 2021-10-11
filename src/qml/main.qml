import QtQuick.Controls 2.3
import ChartQt 1.0

ApplicationWindow {
    visible: true

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
