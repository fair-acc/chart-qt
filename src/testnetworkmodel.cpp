#include "testnetworkmodel.h"

#include <QNetworkReply>
#include <QStringListModel>

#include <QJsonDocument>
#include <QJsonObject>

TestNetworkModel::TestNetworkModel(QObject *parent)
    : QStringListModel{ parent } {
    setStringList({ "Waiting", "for", "the", "sun" });

    QObject::connect(
            &manager, &QNetworkAccessManager::finished, &manager,
            [this](QNetworkReply *reply) {
                const auto      replyContent = reply->readAll();
                QJsonParseError err;
                auto            json = QJsonDocument::fromJson(replyContent, &err);

                if (err.error) {
                    setStringList({ err.errorString(), replyContent });
                } else {
                    setStringList(json.object().keys());
                }
            });

    auto *reply = manager.post(QNetworkRequest(QUrl("https://localhost:42042/test")),
            "first.service;;a.topic;;;rbacToken;");

    QObject::connect(reply, &QIODevice::readyRead,
            this, [=] {
                if (reply->canReadLine()) {
                    auto            replyContent = reply->readLine();
                    QJsonParseError err;
                    auto            json = QJsonDocument::fromJson(replyContent, &err);

                    if (err.error) {
                        setStringList({ err.errorString(), replyContent });
                    } else {
                        setStringList(json.object().keys());
                    }
                }
            });
}
