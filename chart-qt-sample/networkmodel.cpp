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
    QObject::connect(&manager, &QNetworkAccessManager::sslErrors, &manager, [](QNetworkReply *reply, const QList<QSslError> &errors) {
        reply->ignoreSslErrors();
    });
#endif
}

QString NetworkModel::address() const {
    return m_address;
}

void NetworkModel::setAddress(const QString &address) {
    if (m_address != address) {
        m_address = address;
        emit addressChanged();

        beginResetModel();
        m_fields.clear();
        endResetModel();

        getFields();
    }
}

int NetworkModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_fields.size();
}

QVariant NetworkModel::data(const QModelIndex &index, int role) const {
    const auto &f = m_fields[index.row()];
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
    for (int i = 0; i < m_fields.size(); ++i) {
        auto &f     = m_fields[i];
        auto *reply = manager.get(QNetworkRequest(QUrl(m_address + f.name)));

        QObject::connect(reply, &QNetworkReply::finished, this, [=, this] {
            auto replyContent = reply->readAll();
            m_fields[i].value = QString(replyContent);
            emit dataChanged(index(i, 0), index(i, 0), { int(Role::Value) });
        });
    }
}

void NetworkModel::requestSetValue(int idx, const QString &value) {
    const auto &f     = m_fields[idx];
    auto       *reply = manager.post(QNetworkRequest(QUrl(m_address + f.name)), value.toUtf8().data());

    QObject::connect(reply, &QNetworkReply::finished, this, [=, this] {
        if (reply->error() == QNetworkReply::NoError) {
            m_fields[idx].value = value;
            dataChanged(index(idx, 0), index(idx, 0), { int(Role::Value) });
        }
    });
}

void NetworkModel::getFields() {
    auto *reply = manager.post(QNetworkRequest(QUrl(m_address)),
            "first.service;;a.topic;;;rbacToken;");

    QObject::connect(reply, &QNetworkReply::finished, this, [=, this] {
        auto            replyContent = reply->readAll();
        QJsonParseError err;
        auto            json = QJsonDocument::fromJson(replyContent, &err);

        if (!err.error) {
            const auto names = json.array();

            beginResetModel();
            m_fields.clear();
            for (const auto &n : names) {
                m_fields.push_back(Field{ n.toString(), {} });
            }
            endResetModel();
        }
    });
}

} // namespace ChartQtSample
