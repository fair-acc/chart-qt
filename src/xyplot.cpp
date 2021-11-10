#include "xyplot.h"
#include "plot.h"

#include <QOpenGLContext>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGFlatColorMaterial>
#include <QFile>
#include <QQuickWindow>

#include <private/qrhi_p.h>

#ifndef WASM
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_1_Core>
#endif // WASM

#include "sindataset.h"

namespace chart_qt {

XYPlot::XYPlot() {
    auto ds = SinDataSet::instance();
    connect(ds, &DataSet::dataChanged, this, &XYPlot::updateData);

    setStride(3 * sizeof(float));
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::LineStrip);
    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic, 0, QQuick3DGeometry::Attribute::F32Type);

    emit ds->dataChanged(0, 0);
}

void XYPlot::updateData() {
    auto ds = static_cast<DataSet *>(sender());

    constexpr int stride = 3 * sizeof(float);
    const int dataCount = ds->getDataCount();
    const int nBytesRequired = stride * dataCount;
    if (m_vertices.size() < nBytesRequired || m_vertices.size() > nBytesRequired) {
        m_vertices.resize(nBytesRequired);
    }

    const auto data = reinterpret_cast<float *>(m_vertices.data());
    const auto xData = ds->getValues(0).data();
    const auto yData = ds->getValues(1).data();
    for (int i = 0; i < dataCount; ++i) {
        const int index = i * 3;
        data[index + 0] = xData[i]; // x-axis
        data[index + 1] = yData[i]; // y-axis
        //data[index + 2] = 0.0F; // z-axis
    }
    setVertexData(m_vertices);
    update(); // resource leak somewhere in QSGRenderThread: CPU & GPU usage increases slowly over time (N.B. const memory usage), exists since 5.15.2 at least
    // possibly related:
    // https://forum.qt.io/topic/117182/qtquick3d-how-to-customize-geometry-data-and-update-it-in-real-time/5
}

}
