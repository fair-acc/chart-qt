#include "networkmodel.h"

#include <QNetworkReply>
#include <QStringListModel>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace ChartQtSample {

NetworkModel::NetworkModel(QObject *parent)
    : QAbstractListModel{ parent } {
#ifndef QT_NO_SSL
    QObject::connect(&_manager, &QNetworkAccessManager::sslErrors, &_manager, [](QNetworkReply *reply, const QList<QSslError> &errors) {
        reply->ignoreSslErrors();
    });
#endif
}

QString NetworkModel::address() const {
    return _address;
}

void NetworkModel::setAddress(const QString &address) {
    if (_address != address) {
        _address = address;
        emit addressChanged();

        beginResetModel();
        _fields.clear();
        endResetModel();

        getFields();
    }
}

int NetworkModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return _fields.size();
}

QVariant NetworkModel::data(const QModelIndex &index, int role) const {
    const auto &f = _fields[index.row()];
    switch (Role(role)) {
    case Role::Name: return f.name;
    case Role::Value: return f.value;
    }
    return {};
}

QHash<int, QByteArray> NetworkModel::roleNames() const {
    static QHash<int, QByteArray> names = { { int(Role::Name), "name" },
        { int(Role::Value), "value" } };
    return names;
}

void NetworkModel::getValues() {
    for (int i = 0; i < _fields.size(); ++i) {
        auto &f     = _fields[i];
        auto *reply = _manager.get(QNetworkRequest(QUrl(_address + f.name)));

        QObject::connect(reply, &QNetworkReply::finished, this, [=, this] {
            auto replyContent = reply->readAll();
            _fields[i].value  = QString(replyContent);
            emit dataChanged(index(i, 0), index(i, 0), { int(Role::Value) });
        });
    }
}

void NetworkModel::requestSetValue(int idx, const QString &value) {
    const auto &f     = _fields[idx];
    auto       *reply = _manager.post(QNetworkRequest(QUrl(_address + f.name)), value.toUtf8().data());

    QObject::connect(reply, &QNetworkReply::finished, this, [=, this] {
        if (reply->error() == QNetworkReply::NoError) {
            _fields[idx].value = value;
            dataChanged(index(idx, 0), index(idx, 0), { int(Role::Value) });
        }
    });
}

void NetworkModel::getFields() {
    auto *reply = _manager.post(QNetworkRequest(QUrl(_address)),
            "first.service;;a.topic;;;rbacToken;");

    QObject::connect(reply, &QNetworkReply::finished, this, [=, this] {
        auto            replyContent = reply->readAll();
        QJsonParseError err;
        auto            json = QJsonDocument::fromJson(replyContent, &err);

        if (!err.error) {
            const auto names = json.array();

            beginResetModel();
            _fields.clear();
            for (const auto &n : names) {
                _fields.push_back(Field{ n.toString(), {} });
            }
            endResetModel();
        }
    });
}

} // namespace ChartQtSample
