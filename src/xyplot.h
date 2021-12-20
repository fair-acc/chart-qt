#ifndef XYPLOT_H
#define XYPLOT_H

#include "plot.h"

namespace chart_qt {

class Node;

class XYPlot : public Plot
{
public:
    QSGNode *sgNode() override;
    void update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused) override;

private:
    size_t m_allocated = 0;
    Node *m_node = nullptr;
};

}

#endif
