#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringListModel>

#include "devicesmodel.h"
#include "networkmodel.h"
#include "sindataset.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<ExpertUi::NetworkModel>("ExpertUi", 1, 0, "NetworkModel");
    qmlRegisterType<ExpertUi::DevicesModel>("ExpertUi", 1, 0, "DevicesModel");
    qmlRegisterType<ExpertUi::SinDataSet>("ExpertUi", 1, 0, "SinDataSet");

    QQmlApplicationEngine engine(QStringLiteral(":/main.qml"));

    return app.exec();
}
