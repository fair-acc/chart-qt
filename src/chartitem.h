#ifndef CHARTITEM_H
#define CHARTITEM_H

#include <QQuickItem>

namespace chart_qt {

class Plot;

class ChartItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
public:
    ChartItem(QQuickItem *parent = nullptr);
    ~ChartItem();

    Q_INVOKABLE void addPlot(chart_qt::Plot *plot);

    bool paused() const;
    void setPaused(bool paused);

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    void wheelEvent(QWheelEvent *evt) override;

signals:
    void pausedChanged();

private:
    void initPlots();
    void schedulePlotUpdate(Plot *plot);

    std::vector<Plot *> m_plots;
    std::vector<Plot *> m_plotsToInit;
    std::vector<Plot *> m_plotsToUpdate;
    QVector2D m_zoom = { 1, 1 };
    bool m_paused = false;
};

}

#endif // CHARTITEM_H
