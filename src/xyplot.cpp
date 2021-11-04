#include "xyplot.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGFlatColorMaterial>
#include <QSGRenderNode>
#include <QOpenGLExtraFunctions>

#ifndef WASM
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_1_Core>
#endif // WASM

#include <QChart>

#include "sindataset.h"

namespace chart_qt {

XYPlot::XYPlot()
{
    auto ds = new SinDataSet;

    connect(ds, &DataSet::dataChanged, this, &XYPlot::updateSeries);
}

void XYPlot::updateSeries()
{
    if (!m_series) {
        return;
    }

    auto ds = static_cast<DataSet *>(sender());
    
    const int dataCount = ds->getDataCount();
    const auto xdata = ds->getValues(0).data();
    const auto ydata = ds->getValues(1).data();

    QList<QPointF> points;
    for (int i = 0; i < dataCount; ++i) {
        points << QPointF(xdata[i], ydata[i]);
    }
    m_series->replace(points);
}

}
