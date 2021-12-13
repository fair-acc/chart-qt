#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringListModel>

#include "chartitem.h"
#include "xyplot.h"
#include "waterfallplot.h"

#include "testnetworkmodel.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<chart_qt::ChartItem>("ChartQt", 1, 0, "ChartItem");
    qmlRegisterType<chart_qt::XYPlot>("ChartQt", 1, 0, "XYPlot");
    qmlRegisterType<chart_qt::WaterfallPlot>("ChartQt", 1, 0, "WaterfallPlot");
    qmlRegisterUncreatableType<chart_qt::Plot>("ChartQt", 1, 0, "Plot", "");

    qmlRegisterType<TestNetworkModel>("ChartQt", 1, 0, "TestNetworkModel");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
