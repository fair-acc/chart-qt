#include <QGuiApplication>
#include <QQmlApplicationEngine>

// #include "chartitem.h"
#include "xyplot.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    // qmlRegisterType<chart_qt::ChartItem>("ChartQt", 1, 0, "ChartItem");
    qmlRegisterType<chart_qt::XYPlot>("ChartQt", 1, 0, "XYPlot");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
