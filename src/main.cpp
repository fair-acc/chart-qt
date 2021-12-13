#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringListModel>

#include "chartitem.h"
#include "xyplot.h"
#include "waterfallplot.h"
#include "axis.h"

#include "devicesmodel.h"
#include "networkmodel.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<chart_qt::ChartItem>("ChartQt", 1, 0, "ChartItem");
    qmlRegisterType<chart_qt::XYPlot>("ChartQt", 1, 0, "XYPlot");
    qmlRegisterType<chart_qt::WaterfallPlot>("ChartQt", 1, 0, "WaterfallPlot");
    qmlRegisterType<chart_qt::Axis>("ChartQt", 1, 0, "Axis");
    qmlRegisterUncreatableType<chart_qt::Plot>("ChartQt", 1, 0, "Plot", "");

    qmlRegisterType<NetworkModel>("ChartQt", 1, 0, "NetworkModel");
    qmlRegisterType<DevicesModel>("ChartQt", 1, 0, "DevicesModel");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
