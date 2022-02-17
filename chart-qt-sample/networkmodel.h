#ifndef TESTNETWORKMODEL_H
#define TESTNETWORKMODEL_H

#include <QNetworkAccessManager>
#include <QQmlEngine>
#include <QStringListModel>

namespace ChartQtSample {

class NetworkModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
public:
    explicit NetworkModel(QObject *parent = nullptr);

    enum class Role {
        Name,
        Value
    };

    QString                address() const;
    void                   setAddress(const QString &address);

    int                    rowCount(const QModelIndex &parent) const override;
    QVariant               data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void       getValues();
    Q_INVOKABLE void       requestSetValue(int index, const QString &value);

Q_SIGNALS:
    void addressChanged();

private:
    void                  getFields();

    QString               m_address;
    QNetworkAccessManager manager;
    struct Field {
        QString  name;
        QVariant value;
    };
    std::vector<Field> m_fields;
};

} // namespace ChartQtSample

#endif // TESTNETWORKMODEL_H
