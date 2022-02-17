#ifndef CHARTITEM_H
#define CHARTITEM_H

#include <stack>

#include <QQmlEngine>
#include <QQuickItem>

#include "plot.h"

namespace chart_qt {

class Plot;
class Axis;

class ChartItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    QML_ELEMENT
public:
    ChartItem(QQuickItem *parent = nullptr);
    ~ChartItem();

    Q_INVOKABLE void           addPlot(chart_qt::Plot *plot);
    Q_INVOKABLE void           addAxis(Axis *axis);

    bool                       paused() const;
    void                       setPaused(bool paused);

    Q_INVOKABLE void           zoomIn(QRectF area);
    Q_INVOKABLE void           zoomOut(QRectF area);
    Q_INVOKABLE void           undoZoom();

    QRectF                     implicitContentRect() const;
    QRectF                     contentRect() const;
    QRectF                     axisRect(Axis *axis) const;

    void                       setMinimumContentMargins(const QMarginsF &margins);

    const std::vector<Axis *> &axes() const;

    void                       componentComplete() override;

protected:
    void     geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void     updatePolish() override;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

signals:
    void pausedChanged();
    void implicitContentRectChanged();

private:
    void                initPlots();
    void                schedulePlotUpdate(Plot *plot);
    QRectF              sanitizeZoomRect(QRectF rect);
    void                updateAxesRect();

    std::vector<Plot *> m_plots;
    std::vector<Plot *> m_plotsToInit;
    std::vector<Plot *> m_plotsToUpdate;
    bool                m_paused           = false;
    double              m_verticalMargin   = 60;
    double              m_horizontalMargin = 30;

    struct AxisLayout;
    class AxisNode;
    std::vector<std::unique_ptr<AxisLayout>> m_axes;
    std::vector<Axis *>                      m_addedAxes;
    std::stack<QRectF>                       m_zoomHistory;
    QMarginsF                                m_minimumMargins;
};

} // namespace chart_qt

#endif // CHARTITEM_H
