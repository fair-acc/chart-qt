#include "waterfallplot.h"
#include "plot.h"

#include <QOpenGLContext>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGFlatColorMaterial>
#include <QFile>
#include <QQuickWindow>
#include <QDebug>

#include "sindataset.h"

namespace chart_qt {

static const int TexHeight = 500;

WaterfallPlot::WaterfallPlot()
{
    auto ds = SinDataSet::instance();
    connect(ds, &DataSet::dataChanged, this, &WaterfallPlot::updateData);

    setFormat(QQuick3DTextureData::RGBA8);
    setSize({ ds->getDataCount()/10, TexHeight });


    m_data.resize(ds->getDataCount()/10 * TexHeight * sizeof(float));
    m_tex.resize(ds->getDataCount()/10 * TexHeight * 4);
    setTextureData(m_tex);
}

double WaterfallPlot::gradientStart() const
{
    return m_gradient[0];
}

void WaterfallPlot::setGradientStart(double g)
{
    if (m_gradient[0] != g) {
        m_gradient[0] = g;
        updateTex(0, TexHeight);

        emit gradientStartChanged();
    }
}

double WaterfallPlot::gradientEnd() const
{
    return m_gradient[1];
}

void WaterfallPlot::setGradientEnd(double g)
{
    if (m_gradient[1] != g) {
        m_gradient[1] = g;
        updateTex(0, TexHeight);

        emit gradientEndChanged();
    }
}

void WaterfallPlot::updateData()
{
    auto ds = static_cast<DataSet *>(sender());
    const auto dc = ds->getDataCount()/10;

    const auto yData = ds->getValues(1).data();
    memcpy(m_data.data() + dc * m_offset * sizeof(float), yData, dc * sizeof(float));

    updateTex(m_offset, m_offset + 1);

    if (++m_offset == TexHeight) {
        m_offset = 0;
    }
    emit offsetChanged();
}

void WaterfallPlot::updateTex(int start, int end)
{
    const int dc = size().width();

    for (int i = start; i < end; ++i) {
        const auto data = reinterpret_cast<float *>(m_data.data()) + dc * i;
        auto tex = reinterpret_cast<unsigned char *>(m_tex.data()) + (dc * 4) * i;

        for (int x = 0; x < dc; ++x) {
            const auto value = data[x];
            auto texel = &tex[x * 4];
            if (value > m_gradient[1] || value < m_gradient[0]) {
                *reinterpret_cast<uint32_t *>(texel) = 0xff000000;
            } else {
                static const QColor colors[] = { Qt::green, Qt::red };

                const double t = (value - m_gradient[0]) / (m_gradient[1] - m_gradient[0]);
                texel[0] = double(colors[0].red()) * t + double(colors[1].red()) * (1. - t);
                texel[1] = double(colors[0].green()) * t + double(colors[1].green()) * (1. - t);
                texel[2] = double(colors[0].blue()) * t + double(colors[1].blue()) * (1. - t);
                texel[3] = 0xff;
            }
        }
    }
    setTextureData(m_tex);

    update();
}

}
