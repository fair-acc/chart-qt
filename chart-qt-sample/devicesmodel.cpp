#include "devicesmodel.h"

#include <QNetworkReply>
#include <QStringListModel>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace ChartQtSample {

static constexpr auto url = "https://localhost:42042";

DevicesModel::DevicesModel(QObject *parent)
    : QAbstractListModel(parent) {
    _devices.push_back(Device{ "Device 1", url });
}

int DevicesModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return _devices.size() + 1;
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const {
    if (index.row() == _devices.size()) {
        switch (Role(role)) {
        case Role::Name: return tr("Click to create a new connection");
        case Role::Address: return {};
        case Role::Editable: return true;
        }
        return {};
    }

    const auto &d = _devices[index.row()];
    switch (Role(role)) {
    case Role::Name: return d.name;
    case Role::Address: return d.address;
    case Role::Editable: return false;
    }
    return {};
}

QHash<int, QByteArray> DevicesModel::roleNames() const {
    static QHash<int, QByteArray> names = { { int(Role::Name), "name" },
        { int(Role::Address), "address" },
        { int(Role::Editable), "editable" } };
    return names;
}

void DevicesModel::addNewDevice(const QString &address) {
    beginInsertRows({}, _devices.size(), _devices.size());
    _devices.push_back(Device{ tr("Device %1").arg(_devices.size() + 1), address });
    endInsertRows();
}

} // namespace ChartQtSample
