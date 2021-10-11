#ifndef CHARTITEM_H
#define CHARTITEM_H

#include <QQuickItem>

namespace chart_qt {

class Plot;

class ChartItem : public QQuickItem
{
    Q_OBJECT
public:
    ChartItem(QQuickItem *parent = nullptr);
    ~ChartItem();

    Q_INVOKABLE void addPlot(Plot *plot);

protected:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;

private:
    void initPlots();
    void schedulePlotUpdate(Plot *plot);

    std::vector<Plot *> m_plots;
    std::vector<Plot *> m_plotsToInit;
    std::vector<Plot *> m_plotsToUpdate;
};

}

#endif // CHARTITEM_H
