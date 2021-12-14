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

    // auto dataset = new SinDataSet;
    // auto plot = new XYPlot;
    // plot->setDataSet(dataset);
    // addPlot(plot);
}

static SinDataSet *dataset()
{
    static SinDataSet ds;
    return &ds;
}

ChartItem::~ChartItem()
{

}

void ChartItem::addPlot(Plot *plot)
{
    plot->setDataSet(dataset());

    m_plotsToInit.push_back(plot);
    m_plots.push_back(plot);

    connect(plot, &Plot::updateNeeded, this, &QQuickItem::update);

    update();
}

bool ChartItem::paused() const
{
    return m_paused;
}

void ChartItem::setPaused(bool p)
{
    if (m_paused != p) {
        m_paused = p;
        emit pausedChanged();
    }
}

QSGNode *ChartItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    if (!node) {
        node = new QSGTransformNode;
    }

    QMatrix4x4 matrix;
    matrix.scale(m_zoom.x(), m_zoom.y());
    static_cast<QSGTransformNode *>(node)->setMatrix(matrix);

    for (auto p: m_plotsToInit) {
        node->appendChildNode(p->sgNode());
    }
    m_plotsToInit.clear();

    if (!m_paused) {
        for (auto p: m_plots) {
            p->update(window(), width(), height());
        }
    }
    return node;
}

void ChartItem::wheelEvent(QWheelEvent *evt)
{
    double m = evt->angleDelta().y() / 100.;
    if (m < 0) {
        m = 1 / -m;
    }
    m_zoom[0] *= m;
}

}
