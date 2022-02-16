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

#include "dataset.h"
#include "axis.h"
#include "renderutils.h"
#include "xyplotpipeline.h"
#include "errorbarspipeline.h"

namespace chart_qt {

class XYPlot::XYRenderer final : public PlotRenderer
{
public:
    void init()
    {
        m_pipeline.setTopology(Pipeline::Topology::LineStrip);
        m_pipeline.create(this);

        m_xBuffer = createBuffer<XYPlotPipeline::Vx>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::VertexBuffer, 1e6);
        m_yBuffer = createBuffer<XYPlotPipeline::Vy>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::VertexBuffer, 1e6);
        m_ubuf = createBuffer<XYPlotPipeline::Ubo>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::UniformBuffer);

        m_bindingSet = m_pipeline.createBindingSet(this, XYPlotPipeline::Bindings {
            .ubuf = m_ubuf
        });

        m_pipeline.setVxInputBuffer(m_xBuffer);
        m_pipeline.setVyInputBuffer(m_yBuffer);


        m_errorBarsPipeline.setTopology(Pipeline::Topology::Lines);
        m_errorBarsPipeline.create(this);

        m_errorBarsBuffer = createBuffer<ErrorBarsPipeline::Pos>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::VertexBuffer, 2e6);
        m_errorBarsBindingSet = m_errorBarsPipeline.createBindingSet(this, {
            .ubuf = m_ubuf
        });

        m_errorBarsPipeline.setPosInputBuffer(m_errorBarsBuffer);
    }

    void needsUpdate(DataSet *ds)
    {
        m_dataset = ds;
    }

    void updateData()
    {
        const int dataCount = m_dataset->getDataCount();

        const auto xdata = m_dataset->getValues(0).data();
        const auto ydata = m_dataset->getValues(1).data();

        m_xBuffer.update([=](auto *data) {
            memcpy(data, xdata, dataCount * sizeof(float));
        });
        m_yBuffer.update([=](auto *data) {
            memcpy(data, ydata, dataCount * sizeof(float));
        });

        m_allocated = dataCount;

        if (m_dataset->hasErrors) {
            const auto yPosErrors = m_dataset->getPositiveErrors(1).data();
            const auto yNegErrors = m_dataset->getNegativeErrors(1).data();

            m_errorBarsBuffer.update([=](auto *data) {
                for (int i = 0; i < dataCount; ++i) {
                    data[2 * i].pos = QVector2D(xdata[i], ydata[i] - yPosErrors[i]);
                    data[2 * i + 1].pos = QVector2D(xdata[i], ydata[i] + yNegErrors[i]);
                }
            });
        }

        m_dataset = nullptr;
    }

    void prepare() final
    {
        if (!m_pipeline.isCreated()) {
            init();
        }
        if (m_dataset) {
            updateData();
        }
    }

    void render(const QMatrix4x4 &matrix) final
    {
        m_ubuf.update([&](XYPlotPipeline::Ubo *data) {
            auto m = matrix * m_matrix;
            memcpy(data->qt_Matrix.data(), m.data(), 64);
        });

        bindPipeline(m_pipeline);
        bindBindingSet(m_bindingSet);
        draw(m_allocated);

        bindPipeline(m_errorBarsPipeline);
        bindBindingSet(m_errorBarsBindingSet);
        draw(m_allocated * 2);
    }

    XYPlotPipeline m_pipeline;
    size_t m_allocated = 0;
    Buffer<XYPlotPipeline::Vx> m_xBuffer;
    Buffer<XYPlotPipeline::Vy> m_yBuffer;
    Buffer<XYPlotPipeline::Ubo> m_ubuf;
    BindingSet m_bindingSet;

    ErrorBarsPipeline m_errorBarsPipeline;
    Buffer<ErrorBarsPipeline::Pos> m_errorBarsBuffer;
    BindingSet m_errorBarsBindingSet;

    DataSet *m_dataset = nullptr;
    QMatrix4x4 m_matrix;
};

XYPlot::XYPlot()
{
    m_renderer = new XYRenderer;
}

PlotRenderer *XYPlot::renderer()
{
    return m_renderer;
}

void XYPlot::update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused)
{
    QMatrix4x4 m;
    const auto xa = xAxis();
    const auto ya = yAxis();

    const bool xinv = xa && xa->direction() == Axis::Direction::RightToLeft;
    const bool yinv = ya && ya->direction() == Axis::Direction::BottomToTop;

    double xscale = xa ? (xinv ? -chartRect.width() : chartRect.width()) / (xa->max() - xa->min()) : 1;
    double yscale = ya ? (yinv ? -chartRect.height() : chartRect.height()) / (ya->max() - ya->min()) : 1;
    m.scale(xscale, yscale);

    double xtr = xa ? (xinv ? -xa->max() : -xa->min()) : 0;
    double ytr = ya ? (yinv ? -ya->max() : -ya->min()) : 0;
    m.translate(xtr, ytr);

    m_renderer->m_matrix = m;
    if (m_needsUpdate && !paused) {
        m_needsUpdate = false;

        m_renderer->m_dataset = dataSet();
    }
}

}
