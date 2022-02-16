#include "waterfallplot.h"
#include "plot.h"

#include <QSGRenderNode>
#include <QFile>
#include <QQuickWindow>

#include "dataset.h"
#include "axis.h"
#include "waterfallpipeline.h"

namespace chart_qt {

static constexpr int TexHeight = 500;

class WaterfallPlot::Renderer final : public PlotRenderer
{
public:
    void init()
    {
        m_pipeline.setTopology(Pipeline::Topology::TriangleStrip);
        m_pipeline.create(this);

        m_buffer = createBuffer<WaterfallPipeline::Vertex>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::VertexBuffer, 4);
        m_ubuf = createBuffer<WaterfallPipeline::Ubo>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::UniformBuffer);
        m_texture = createTexture<TextureFormat::R32F>({ 10000, TexHeight });

        m_bindingSet = m_pipeline.createBindingSet(this, {
            .ubuf = m_ubuf,
            .tex = m_texture
        });

        m_pipeline.setVertexInputBuffer(m_buffer);
    }

    void needsUpdate(DataSet *ds)
    {
        m_dataset = ds;
    }

    void updateData()
    {
        const int dataCount = m_dataset->getDataCount();
        const auto ydata = m_dataset->getValues(1).data();

        if (m_lineData.size() < 10000) {
            m_lineData.resize(10000);
        }

        for (int i = 0; i < 10000; ++i) {
            m_lineData[i] = ydata[i * 10];
        }

        const auto xdata = m_dataset->getValues(0).data();
        m_dataWidth = xdata[dataCount - 1];

        updateTexture(m_texture, QRect(0, m_lineOffset, 10000, 1), m_lineData.data());
        if (++m_lineOffset == TexHeight) {
            m_lineOffset = 0;
        }

        float startU = m_xaxis[0] / m_dataWidth;
        float endU = m_xaxis[1] / m_dataWidth;

        m_buffer.update([=](auto *data) {
            static const QVector2D vertices[] = {
                { 0, 0 },
                { 0, 1 },
                { 1, 0 },
                { 1, 1 },
            };
            for (int i = 0; i < 4; ++i) {
                data[i].vertex = vertices[i];
            }

            data[0].uv_in = { startU, 0 };
            data[1].uv_in = { startU, 1 };
            data[2].uv_in = { endU, 0 };
            data[3].uv_in = { endU, 1 };
        });

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
        QMatrix4x4 m = matrix;
        m.scale(rect().width(), rect().height());

        m_ubuf.update([&](WaterfallPipeline::Ubo *data) {
            memcpy(data->qt_Matrix.data(), m.data(), 64);
            data->gradient = m_gradient;
            data->lineOffset = m_lineOffset;
        });

        bindPipeline(m_pipeline);
        bindBindingSet(m_bindingSet);
        draw(4);
    }

    void setGradient(float start, float stop)
    {
        m_gradient = { start, stop };
    }

    WaterfallPipeline m_pipeline;
    Buffer<WaterfallPipeline::Vertex> m_buffer;
    Buffer<WaterfallPipeline::Ubo> m_ubuf;
    Texture<TextureFormat::R32F> m_texture;
    BindingSet m_bindingSet;

    DataSet *m_dataset = nullptr;
    QVector2D m_gradient = {0, 1};
    int m_lineOffset = 0;
    float m_xaxis[2] = { 0, 1 };
    float m_dataWidth = 1;
    std::vector<float> m_lineData;
};

WaterfallPlot::WaterfallPlot()
{
    m_renderer = new Renderer;
}

PlotRenderer *WaterfallPlot::renderer()
{
    return m_renderer;
}

void WaterfallPlot::update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused)
{
    if (auto xa = xAxis()) {
        m_renderer->m_xaxis[0] = xa->min();
        m_renderer->m_xaxis[1] = xa->max();
    }
    m_renderer->setGradient(m_gradientStart, m_gradientStop);
    if (m_needsUpdate && !paused) {
        m_needsUpdate = false;

        m_renderer->m_dataset = dataSet();
    }
}

double WaterfallPlot::gradientStart() const
{
    return m_gradientStart;
}

void WaterfallPlot::setGradientStart(double g)
{
    if (m_gradientStart != g) {
        m_gradientStart = g;
        emit gradientChanged();
    }
}

double WaterfallPlot::gradientStop() const
{
    return m_gradientStop;
}

void WaterfallPlot::setGradientStop(double g)
{
    if (m_gradientStop != g) {
        m_gradientStop = g;
        emit gradientChanged();
    }
}

}
