import QtQuick.Controls 2.3
import ChartQt 1.0
import QtQuick3D
import QtQuick

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

    Item {
        id: gradient
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.verticalCenter
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
            position: Qt.vector3d(-1000, 200, 0)
            geometry: XYPlot {}
            scale: Qt.vector3d(20, 200, 1)
            materials: [ DefaultMaterial {
                    diffuseColor: "red"
                }
            ]
        }


        Model {
            source: "#Rectangle"
            scale: Qt.vector3d(20, 4, 1)
            position: Qt.vector3d(0, -200, 0)
            materials: [
                CustomMaterial {
                    vertexShader: "waterfall.vert"
                    fragmentShader: "waterfall.frag"
                    property int offset: t.offset
                    property TextureInput tex: TextureInput {
                        texture: Texture {
                            textureData: WaterfallPlotTexture {
                                id: t
                                gradientStart: 1 - (bottomHandle.y + bottomHandle.height / 2) / gradient.height
                                gradientEnd: 1 - (topHandle.y + topHandle.height / 2) / gradient.height
                            }
                        }
                    }
                }
            ]
        }
    }
}
