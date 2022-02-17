#ifndef CHARTQT_CHARTLAYOUT_H
#define CHARTQT_CHARTLAYOUT_H

#include <QQmlListProperty>
#include <QQuickItem>

namespace chart_qt {

class ChartItem;

class ChartLayout : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<ChartItem> charts READ charts)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_CLASSINFO("DefaultProperty", "charts")
    QML_ELEMENT
public:
    explicit ChartLayout(QQuickItem *parent = nullptr);
    ~ChartLayout() noexcept;

    QQmlListProperty<ChartItem> charts();

    Qt::Orientation             orientation() const;
    void                        setOrientation(Qt::Orientation o);

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry);
    void updatePolish() override;

Q_SIGNALS:
    void orientationChanged();

private:
    std::vector<ChartItem *> m_charts;
    Qt::Orientation          m_orientation = Qt::Horizontal;
};

} // namespace chart_qt

#endif
