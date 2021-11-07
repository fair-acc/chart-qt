#ifndef XYPLOT_H
#define XYPLOT_H

#include <QQuick3DGeometry>

namespace chart_qt {

class XYPlot : public QQuick3DGeometry {
    QByteArray m_vertices;
public:
    XYPlot();

private:
    void updateData();
};

}

#endif
