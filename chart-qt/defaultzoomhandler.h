#ifndef CHARTQT_DEFAULTCHARTINPUTHANDLER_H
#define CHARTQT_DEFAULTCHARTINPUTHANDLER_H

#include <QColor>
#include <QObject>
#include <QPointF>
#include <QQmlEngine>
#include <QQmlParserStatus>
#include <QRectF>

class QWheelEvent;
class QMouseEvent;
class QTouchEvent;
class QQuickItem;

namespace chart_qt {

class Axis;
class ChartItem;

class DefaultZoomHandler : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQuickItem *zoomRectangle READ zoomRectangle WRITE setZoomRectangle NOTIFY zoomRectangleChanged)
    QML_ELEMENT
public:
    DefaultZoomHandler(QObject *parent = nullptr);

    QQuickItem *zoomRectangle() const;
    void        setZoomRectangle(QQuickItem *rect);

    void        classBegin() override;
    void        componentComplete() override;

    bool        eventFilter(QObject *object, QEvent *event) override;

Q_SIGNALS:
    void zoomRectangleChanged();

private:
    ChartItem          *chartItem() const;
    void                wheelEvent(QWheelEvent *evt);
    void                mousePressEvent(QMouseEvent *evt);
    void                mouseMoveEvent(QMouseEvent *evt);
    void                mouseReleaseEvent(QMouseEvent *evt);
    void                touchEvent(QTouchEvent *evt);
    void                startPanning(const QPointF &pos);
    void                pan(const QPointF &pos);

    QPointF             _pressPos;
    bool                _panning : 1 = false;
    std::vector<Axis *> _panningAxis;
    QQuickItem         *_zoomRectangle = nullptr;
    QPointF             _pinchPoints[2];
};

} // namespace chart_qt

#endif
