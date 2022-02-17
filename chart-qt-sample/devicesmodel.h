#ifndef DEVICESMODEL_H
#define DEVICESMODEL_H

#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <QStringListModel>

namespace ChartQtSample {

class DevicesModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit DevicesModel(QObject *parent = nullptr);

    enum class Role {
        Name,
        Address,
        Editable,
    };

    int                    rowCount(const QModelIndex &parent) const override;
    QVariant               data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void       addNewDevice(const QString &address);

private:
    struct Device {
        QString name;
        QString address;
    };
    std::vector<Device> _devices;
};

} // namespace ChartQtSample

#endif // TESTNETWORKMODEL_H
