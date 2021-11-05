#include "xyplot.h"
#include "plot.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGFlatColorMaterial>
#include <QSGRenderNode>
#include <QOpenGLExtraFunctions>
#include <QFile>
#include <QQuickWindow>

#include <QSGRendererInterface>
#include <private/qrhi_p.h>

#ifndef WASM
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_1_Core>
#endif // WASM

#include "sindataset.h"

namespace chart_qt {

XYPlot::XYPlot()
{
    auto ds = new SinDataSet;
    connect(ds, &DataSet::dataChanged, this, &XYPlot::updateData);

    setPrimitiveType(QQuick3DGeometry::PrimitiveType::LineStrip);
    setStride(3 * sizeof(float));
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);

    emit ds->dataChanged(0, 0);
}

void XYPlot::updateData()
{
    auto ds = static_cast<DataSet *>(sender());
    QByteArray vertices;

    const int dataCount = ds->getDataCount();

    const auto xdata = ds->getValues(0).data();
    const auto ydata = ds->getValues(1).data();

    for (int i = 0; i < dataCount; ++i) {
        vertices.append(reinterpret_cast<const char *>(&xdata[i]), 4);
        vertices.append(reinterpret_cast<const char *>(&ydata[i]), 4);
        static const float zero = 0.f;
        vertices.append(reinterpret_cast<const char *>(&zero), 4);
    }

    setVertexData(vertices);

    update();
}

}
