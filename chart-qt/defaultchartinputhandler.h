#ifndef CHARTQT_DEFAULTCHARTINPUTHANDLER_H
#define CHARTQT_DEFAULTCHARTINPUTHANDLER_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQmlEngine>
#include <QPointF>
#include <QRectF>
#include <QColor>

class QWheelEvent;
class QMouseEvent;
class QTouchEvent;
class QQuickItem;

namespace chart_qt {

class Axis;
class ChartItem;

class DefaultChartInputHandler : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQuickItem *zoomRectangle READ zoomRectangle WRITE setZoomRectangle NOTIFY zoomRectangleChanged)
    QML_ELEMENT
public:
    DefaultChartInputHandler(QObject *parent = nullptr);

    QQuickItem *zoomRectangle() const;
    void setZoomRectangle(QQuickItem *rect);

    void classBegin() override;
    void componentComplete() override;

    bool eventFilter(QObject *object, QEvent *event) override;

Q_SIGNALS:
    void zoomRectangleChanged();

private:
    ChartItem *chartItem() const;
    void wheelEvent(QWheelEvent *evt);
    void mousePressEvent(QMouseEvent *evt);
    void mouseMoveEvent(QMouseEvent *evt);
    void mouseReleaseEvent(QMouseEvent *evt);
    void touchEvent(QTouchEvent *evt);
    void startPanning(const QPointF &pos);
    void pan(const QPointF &pos);

    QPointF m_pressPos;
    bool m_panning: 1 = false;
    std::vector<Axis *> m_panningAxis;
    QQuickItem *m_zoomRectangle = nullptr;
    QPointF m_pinchPoints[2];

};

}

#endif
