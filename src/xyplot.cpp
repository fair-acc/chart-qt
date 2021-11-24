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

#include "dataset.h"

namespace chart_qt {

class Node : public QSGRenderNode
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

        m_pipeline->setTopology(QRhiGraphicsPipeline::Topology::LineStrip);

        m_xbuffer = rhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer, 1e6);
        m_xbuffer->create();

        m_ybuffer = rhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer, 1e6);
        m_ybuffer->create();

        m_ubuf = rhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UsageFlag::UniformBuffer, 16 * sizeof(float));
        m_ubuf->create();

        QFile vs_source(":/src/shaders/xyplot_float.vert.qsb");
        vs_source.open(QIODevice::ReadOnly);

        QFile fs_source(":/src/shaders/xyplot_float.frag.qsb");
        fs_source.open(QIODevice::ReadOnly);

        QShader vshader = QShader::fromSerialized(vs_source.readAll());
        QShader fshader = QShader::fromSerialized(fs_source.readAll());

        m_pipeline->setShaderStages({ { QRhiShaderStage::Type::Vertex, vshader }, { QRhiShaderStage::Type::Fragment, fshader } });
        m_pipeline->setRenderPassDescriptor(renderPassDescriptor());

        m_resourceBindings = rhi->newShaderResourceBindings();
        const auto uniforms = vshader.description().uniformBlocks();
        std::vector<QRhiShaderResourceBinding> bindings;
        for (auto &u: uniforms) {
            bindings.push_back(QRhiShaderResourceBinding::uniformBuffer(u.binding, QRhiShaderResourceBinding::StageFlag::VertexStage, m_ubuf));
        }
        m_resourceBindings->setBindings(bindings.begin(), bindings.end());
        m_resourceBindings->create();
        m_pipeline->setShaderResourceBindings(m_resourceBindings);

        QRhiVertexInputLayout layout;
        layout.setBindings({ { sizeof(float) }, { sizeof(float) }});
        layout.setAttributes({ { /*buffer binding*/ 0, /*location*/ 0, QRhiVertexInputAttribute::Format::Float, /*offset*/ 0 },
                               { /*buffer binding*/ 1, /*location*/ 1, QRhiVertexInputAttribute::Format::Float, /*offset*/ 0 } });


        m_pipeline->setVertexInputLayout(layout);

        m_pipeline->create();
    }

    void releaseResources() override
    {
        delete m_ubuf;
        delete m_xbuffer;
        delete m_ybuffer;

        delete m_resourceBindings;
        delete m_pipeline;
        m_pipeline = nullptr;
    }

    QSGRenderNode::RenderingFlags flags() const override
    {
        // By returning NoExternalRendering the scene graph renderer doesn't call beginExternal()
        // endExternal() around the call to render(), which we need given we're not calling directly
        // OpenGL/Vulkan stuff, but we're only going through the QRhi interface
        return QSGRenderNode::NoExternalRendering;
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

        auto data = m_xbuffer->beginFullDynamicBufferUpdateForCurrentFrame();
        auto xdst = reinterpret_cast<float *>(data);

        memcpy(xdst, xdata, dataCount * sizeof(float));
        m_xbuffer->endFullDynamicBufferUpdateForCurrentFrame();

        auto ydst = reinterpret_cast<float *>(m_ybuffer->beginFullDynamicBufferUpdateForCurrentFrame());
        memcpy(ydst, ydata, dataCount * sizeof(float));
        m_ybuffer->endFullDynamicBufferUpdateForCurrentFrame();

        m_allocated = dataCount;
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

    void render(const QSGRenderNode::RenderState *state) final
    {
        QMatrix4x4 m = m_matrix;
        m.translate(0, m_rect.height() / 2);
        m.scale(20, 200);
        m = (*state->projectionMatrix()) * m;

        auto *ubuf = reinterpret_cast<float *>(m_ubuf->beginFullDynamicBufferUpdateForCurrentFrame());
        memcpy(ubuf, m.data(), 16 * sizeof(float));
        m_ubuf->endFullDynamicBufferUpdateForCurrentFrame();

        auto [size, cmdbuf] = getRenderResources();

        cmdbuf->setGraphicsPipeline(m_pipeline);
        const QRhiCommandBuffer::VertexInput bindings[] = { { m_xbuffer, 0 }, { m_ybuffer, 0 } };
        cmdbuf->setVertexInput(0, 2, bindings);
        cmdbuf->setShaderResources(m_resourceBindings);
        cmdbuf->setViewport(QRhiViewport(0, 0, size.width(), size.height()));
        cmdbuf->draw(m_allocated);
    }

    void setWindow(QQuickWindow *win, const QSize &rect)
    {
        m_window = win;
        m_rect = rect;
    }

    QRhiGraphicsPipeline *m_pipeline = nullptr;
    QRhiShaderResourceBindings *m_resourceBindings;

    QQuickWindow *m_window;
    QRhiBuffer *m_xbuffer;
    QRhiBuffer *m_ybuffer;
    QRhiBuffer *m_ubuf;
    size_t m_allocated = 0;
    QMatrix4x4 m_matrix;
    QSize m_rect;
    DataSet *m_dataset = nullptr;
};

QSGNode *XYPlot::sgNode()
{
    if (!m_node) {
        m_node = new Node;
    }
    return m_node;
}

void XYPlot::update(QQuickWindow *window, double w, double h)
{
    m_node->setWindow(window, QSize(w, h));
    if (m_needsUpdate) {
        m_needsUpdate = false;

        m_node->needsUpdate(dataSet());
        m_node->markDirty(QSGNode::DirtyGeometry);
    }
}

}
