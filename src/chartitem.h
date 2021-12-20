#ifndef CHARTITEM_H
#define CHARTITEM_H

#include <stack>

#include <QQuickItem>

namespace chart_qt {

class Plot;
class Axis;
class AxisNode;

class ChartItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(QColor zoomRectColor READ zoomRectColor WRITE setZoomRectColor NOTIFY zoomRectColorChanged)
public:
    ChartItem(QQuickItem *parent = nullptr);
    ~ChartItem();

    Q_INVOKABLE void addPlot(chart_qt::Plot *plot);
    Q_INVOKABLE void addAxis(Axis *axis);

    bool paused() const;
    void setPaused(bool paused);

    QColor zoomRectColor() const;
    void setZoomRectColor(const QColor &c);

    Q_INVOKABLE void zoomIn(QRectF area);
    Q_INVOKABLE void zoomOut(QRectF area);

    void componentComplete() override;

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    void wheelEvent(QWheelEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;

signals:
    void pausedChanged();
    void zoomRectColorChanged();

private:
    void initPlots();
    void schedulePlotUpdate(Plot *plot);
    QRectF sanitizeZoomRect(QRectF rect);

    std::vector<Plot *> m_plots;
    std::vector<Plot *> m_plotsToInit;
    std::vector<Plot *> m_plotsToUpdate;
    bool m_paused = false;
    double m_verticalMargin = 60;
    double m_horizontalMargin = 30;

    struct AxisLayout;
    struct AxisNode;
    std::vector<std::unique_ptr<AxisLayout>> m_axes;
    QPointF m_pressPos;
    bool m_panning: 1 = false;
    bool m_drawingZoomSquare: 1 = false;
    QColor m_zoomRectColor = QColor(90, 200, 250, 150);
    QRectF m_zoomRect;
    std::stack<QRectF> m_zoomHistory;
};

}

#endif // CHARTITEM_H
