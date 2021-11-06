import QtQuick.Controls 2.3
import ChartQt 1.0
import QtQuick3D

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

    View3D {
        id: view
        anchors.fill: parent

        OrthographicCamera {
            position: Qt.vector3d(0, 0, 1000)
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Model {
            position: Qt.vector3d(-1000, 0, 0)
            geometry: XYPlot {}
            scale: Qt.vector3d(20, 200, 1)
            materials: [ DefaultMaterial {
                    diffuseColor: "red"
                }
            ]
        }
    }
}
