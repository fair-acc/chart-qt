#include <QApplication>
#include <QQmlApplicationEngine>

#include "xyplot.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    qmlRegisterType<chart_qt::XYPlot>("ChartQt", 1, 0, "XYPlotUpdater");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
