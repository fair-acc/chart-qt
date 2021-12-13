#ifndef TESTNETWORKMODEL_H
#define TESTNETWORKMODEL_H

#include <QNetworkAccessManager>
#include <QStringListModel>
#include <QQmlEngine>

class TestNetworkModel : public QStringListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit TestNetworkModel(QObject *parent = nullptr);

    using QStringListModel::setStringList;

private:
    QNetworkAccessManager manager;

};

#endif // TESTNETWORKMODEL_H
