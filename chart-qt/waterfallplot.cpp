#include "waterfallplot.h"
#include "plot.h"

#include <QSGRenderNode>
#include <QFile>
#include <QQuickWindow>

#include <QSGRendererInterface>
#include <private/qrhi_p.h>

#include "dataset.h"
#include "axis.h"

namespace chart_qt {

static constexpr int TexHeight = 500;

class WaterfallPlot::Node : public QSGRenderNode
{
public:
    Node()
    {
    }

    QRhiRenderPassDescriptor *renderPassDescriptor() const
    {
        auto ri = m_window->rendererInterface();
        auto swapchain = static_cast<QRhiSwapChain *>(ri->getResource(m_window,
                                                                      QSGRendererInterface::RhiSwapchainResource));
        if (swapchain) {
            return swapchain->renderPassDescriptor();
        }

        auto target = static_cast<QRhiRenderTarget *>(ri->getResource(m_window,
                                                                      QSGRendererInterface::RhiRedirectRenderTarget));
        return target->renderPassDescriptor();
    }

    void init()
    {
        auto ri = m_window->rendererInterface();
        auto rhi = static_cast<QRhi *>(ri->getResource(m_window, QSGRendererInterface::RhiResource));

        m_pipeline = rhi->newGraphicsPipeline();

        m_pipeline->setTopology(QRhiGraphicsPipeline::Topology::TriangleStrip);

        m_vbuf = rhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer, 20 * sizeof(float));
        m_vbuf->create();

        m_ubuf = rhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UsageFlag::UniformBuffer, 19 * sizeof(float));
        m_ubuf->create();

        m_texture = rhi->newTexture(QRhiTexture::Format::R32F, QSize(10000, TexHeight));
        m_texture->create();

        m_sampler = rhi->newSampler(QRhiSampler::Filter::Nearest, QRhiSampler::Filter::Nearest, QRhiSampler::Filter::None,
                                    QRhiSampler::AddressMode::ClampToEdge, QRhiSampler::AddressMode::Repeat);
        m_sampler->create();

        QFile vs_source(":/shaders/waterfall.vert.qsb");
        vs_source.open(QIODevice::ReadOnly);

        QFile fs_source(":/shaders/waterfall.frag.qsb");
        fs_source.open(QIODevice::ReadOnly);

        QShader vshader = QShader::fromSerialized(vs_source.readAll());
        QShader fshader = QShader::fromSerialized(fs_source.readAll());

        m_pipeline->setShaderStages({ { QRhiShaderStage::Type::Vertex, vshader }, { QRhiShaderStage::Type::Fragment, fshader } });
        m_pipeline->setRenderPassDescriptor(renderPassDescriptor());

        m_resourceBindings = rhi->newShaderResourceBindings();
        const auto uniforms = vshader.description().uniformBlocks();
        std::vector<QRhiShaderResourceBinding> bindings;
        for (auto &u: uniforms) {
            bindings.push_back(QRhiShaderResourceBinding::uniformBuffer(u.binding, QRhiShaderResourceBinding::StageFlag::VertexStage |
                                                                                   QRhiShaderResourceBinding::StageFlag::FragmentStage,
                                                                        m_ubuf));
        }
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::StageFlag::FragmentStage, m_texture, m_sampler));
        m_resourceBindings->setBindings(bindings.begin(), bindings.end());
        m_resourceBindings->create();
        m_pipeline->setShaderResourceBindings(m_resourceBindings);

        QRhiVertexInputLayout layout;
        layout.setBindings({ { /*stride*/ 2 * sizeof(float) }, { 2 * sizeof(float) } });
        layout.setAttributes({ { /*buffer binding*/ 0, /*location*/ 0, QRhiVertexInputAttribute::Format::Float2, /*offset*/ 0 },
                               { /*buffer binding*/ 0, /*location*/ 1, QRhiVertexInputAttribute::Format::Float2, /*offset*/ 10 * sizeof(float) } });

        m_pipeline->setVertexInputLayout(layout);

        m_pipeline->create();

        static const float vertices[] = {
            0, 0,
            0, 1,
            1, 1,
            0, 0,
            1, 0,

            0, 0,
            0, 1,
            1, 1,
            0, 0,
            1, 0
        };
        m_updateBatch = rhi->nextResourceUpdateBatch();
        m_updateBatch->updateDynamicBuffer(m_vbuf, 0, sizeof(vertices), vertices);
    }

    void releaseResources() override
    {
        delete m_ubuf;
        delete m_vbuf;

        delete m_resourceBindings;
        delete m_pipeline;
        m_pipeline = nullptr;
    }

    QSGRenderNode::RenderingFlags flags() const override
    {
        // By returning NoExternalRendering the scene graph renderer doesn't call beginExternal()
        // endExternal() around the call to render(), which we need given we're not calling directly
        // OpenGL/Vulkan stuff, but we're only going through the QRhi interface
        return QSGRenderNode::DepthAwareRendering | QSGRenderNode::NoExternalRendering;
    }

    void needsUpdate(DataSet *ds)
    {
        m_dataset = ds;
    }

    std::tuple<QSize, QRhiCommandBuffer *> getRenderResources()
    {
        auto ri = m_window->rendererInterface();
        auto swapchain = static_cast<QRhiSwapChain *>(ri->getResource(m_window,
                                                                      QSGRendererInterface::RhiSwapchainResource));

        if (swapchain) {
            return { swapchain->currentPixelSize(), swapchain->currentFrameCommandBuffer() };
        }

        auto cmdbuf = static_cast<QRhiCommandBuffer *>(ri->getResource(m_window,
                                                                       QSGRendererInterface::RhiRedirectCommandBuffer));
        auto target = static_cast<QRhiRenderTarget *>(ri->getResource(m_window,
                                                                      QSGRendererInterface::RhiRedirectRenderTarget));

        return { target->pixelSize(), cmdbuf };
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

        QRhiTextureSubresourceUploadDescription subres(m_lineData.data(), 10000 * 4);
        subres.setDataStride(m_lineData.size() * 4);
        subres.setDestinationTopLeft({0, m_lineOffset });
        subres.setSourceTopLeft({ 0, 0 });
        subres.setSourceSize({ 10000, 1 });

        if (++m_lineOffset == TexHeight) {
            m_lineOffset = 0;
        }

        if (!m_updateBatch) {
            auto ri = m_window->rendererInterface();
            auto rhi = static_cast<QRhi *>(ri->getResource(m_window, QSGRendererInterface::RhiResource));
            m_updateBatch = rhi->nextResourceUpdateBatch();
        }
        m_updateBatch->uploadTexture(m_texture, QRhiTextureUploadEntry(0, 0, subres));

        m_dataset = nullptr;
    }

    void prepare() final
    {
        // matrix() returns a dangling pointer in render(), so copy it here for later.
        // See https://bugreports.qt.io/browse/QTBUG-97589
        m_matrix = *matrix();

        if (!m_pipeline) {
            init();
        }

        if (m_dataset) {
            updateData();
        }

        auto [size, cmdbuf] = getRenderResources();
        cmdbuf->resourceUpdate(m_updateBatch);
        m_updateBatch = nullptr;
    }

    void render(const QSGRenderNode::RenderState *state) final
    {
        QMatrix4x4 m = m_matrix;
        m.scale(m_rect.width(), m_rect.height());
        m = (*state->projectionMatrix()) * m;

        float startU = m_xaxis[0] / m_dataWidth;
        float endU = m_xaxis[1] / m_dataWidth;

        static const float vertices[] = {
            0, 0,
            0, 1,
            1, 1,
            0, 0,
            1, 0,

            0, 0,
            0, 1,
            1, 1,
            0, 0,
            1, 0
        };

        auto uv = reinterpret_cast<float *>(m_vbuf->beginFullDynamicBufferUpdateForCurrentFrame()) + 10;
#ifdef WASM
        memcpy(uv - 10, vertices, 80);
#endif
        uv[0] = startU;
        uv[2] = startU;
        uv[4] = endU;
        uv[6] = startU;
        uv[8] = endU;
        m_vbuf->endFullDynamicBufferUpdateForCurrentFrame();

        auto *ubuf = reinterpret_cast<float *>(m_ubuf->beginFullDynamicBufferUpdateForCurrentFrame());
        memcpy(ubuf, m.data(), 16 * sizeof(float));
        memcpy(ubuf + 16, m_gradient, 2 * sizeof(float));
        *reinterpret_cast<int *>(&ubuf[18]) = m_lineOffset;

        m_ubuf->endFullDynamicBufferUpdateForCurrentFrame();

        auto [size, cmdbuf] = getRenderResources();

        cmdbuf->setGraphicsPipeline(m_pipeline);
        const QRhiCommandBuffer::VertexInput bindings[] = { { m_vbuf, 0 }, { m_vbuf, 40} };
        cmdbuf->setVertexInput(0, 2, bindings);
        cmdbuf->setShaderResources(m_resourceBindings);
        cmdbuf->setViewport(QRhiViewport(0, 0, size.width(), size.height()));
        cmdbuf->draw(5);
    }

    void setWindow(QQuickWindow *win, const QSize &rect)
    {
        m_window = win;
        m_rect = rect;
    }

    void setGradient(double start, double stop)
    {
        m_gradient[0] = start;
        m_gradient[1] = stop;
    }

    QRhiGraphicsPipeline *m_pipeline = nullptr;
    QRhiShaderResourceBindings *m_resourceBindings;

    QQuickWindow *m_window;
    QRhiBuffer *m_vbuf = nullptr;
    QRhiResourceUpdateBatch *m_updateBatch = nullptr;
    QRhiTexture *m_texture;
    QRhiSampler *m_sampler;
    QRhiBuffer *m_ubuf;
    size_t m_allocated = 0;
    QMatrix4x4 m_matrix;
    QSize m_rect;
    DataSet *m_dataset = nullptr;
    int m_lineOffset = 0;
    double m_dataWidth = 1;
    std::vector<float> m_lineData;
    float m_gradient[2] = { 0, 0 };
    float m_xaxis[2] = { 0, 1 };
};

QSGNode *WaterfallPlot::sgNode()
{
    if (!m_node) {
        m_node = new Node;
    }
    return m_node;
}

void WaterfallPlot::update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused)
{
    if (auto xa = xAxis()) {
        m_node->m_xaxis[0] = xa->min();
        m_node->m_xaxis[1] = xa->max();
    }
    m_node->setWindow(window, chartRect.size());
    m_node->setGradient(m_gradientStart, m_gradientStop);
    if (m_needsUpdate && !paused) {
        m_needsUpdate = false;

        m_node->needsUpdate(dataSet());
        m_node->markDirty(QSGNode::DirtyGeometry);
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
