#include "chartitem.h"

#include <QSGTransformNode>

#include "plot.h"

#include "sindataset.h"
#include "xyplot.h"

namespace chart_qt {

ChartItem::ChartItem(QQuickItem *parent)
         : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);

    auto dataset = new SinDataSet;
    auto plot = new XYPlot;
    plot->setDataSet(dataset);
    addPlot(plot);
}

ChartItem::~ChartItem()
{

}

void ChartItem::addPlot(Plot *plot)
{
    m_plotsToInit.push_back(plot);
    m_plots.push_back(plot);

    connect(plot, &Plot::updateNeeded, this, &QQuickItem::update);

    update();
}

QSGNode *ChartItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    if (!node) {
        node = new QSGTransformNode;
    }

    QMatrix4x4 matrix;
    matrix.translate(x(), y());
    static_cast<QSGTransformNode *>(node)->setMatrix(matrix);

    for (auto p: m_plotsToInit) {
        node->appendChildNode(p->sgNode());
    }
    m_plotsToInit.clear();

    for (auto p: m_plots) {
        p->update(window(), width(), height());
    }
    return node;
}

}
