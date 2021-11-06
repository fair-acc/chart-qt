import QtQuick.Controls 2.3
import ChartQt 1.0
import QtCharts 2.0
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

    ChartView {
        id: chart
        anchors.fill: parent
        axes: [xAxis, yAxis]

        ValueAxis {
            id: xAxis
            min: 0
            max: 70
        }

        ValueAxis {
            id: yAxis
            min: -2
            max: 2
        }

        LineSeries {
            axisX: xAxis
            axisY: yAxis
            id: plot
            useOpenGL: true

            XYPlotUpdater {
                series: plot

            }
        }
    }
}
