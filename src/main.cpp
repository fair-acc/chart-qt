#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include "chartitem.h"
#include "xyplot.h"
#include "waterfallplot.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<chart_qt::ChartItem>("ChartQt", 1, 0, "ChartItem");
    qmlRegisterType<chart_qt::XYPlot>("ChartQt", 1, 0, "XYPlot");
    qmlRegisterType<chart_qt::WaterfallPlot>("ChartQt", 1, 0, "WaterfallPlot");
    qmlRegisterUncreatableType<chart_qt::Plot>("ChartQt", 1, 0, "Plot", "");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
