#ifndef XYPLOT_H
#define XYPLOT_H

#include <QQuick3DGeometry>

#include "plot.h"

namespace chart_qt {

class Node;

class XYPlot : public QQuick3DGeometry
{
public:
    XYPlot();
    // QSGNode *sgNode() override;
    // void update(QQuickWindow *window, double w, double h) override;

private:
    void updateData();

    size_t m_allocated = 0;
    // Node *m_node = nullptr;
};

}

#endif
