#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringListModel>

#include "devicesmodel.h"
#include "networkmodel.h"
#include "sindataset.h"

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);

    qmlRegisterType<ChartQtSample::NetworkModel>("ChartQtSample", 1, 0, "NetworkModel");
    qmlRegisterType<ChartQtSample::DevicesModel>("ChartQtSample", 1, 0, "DevicesModel");
    qmlRegisterType<ChartQtSample::SinDataSet>("ChartQtSample", 1, 0, "SinDataSet");
    qmlRegisterType<ChartQtSample::SinDataSet2D>("ChartQtSample", 1, 0, "SinDataSet2D");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
