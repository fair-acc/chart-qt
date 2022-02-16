#include "renderutils.h"

#include <QFile>
#include <QQuickWindow>
#include <QSGRenderNode>
#include <QSGRendererInterface>
#include <private/qrhi_p.h>

namespace chart_qt {

struct Pipeline::Private
{
    QRhiGraphicsPipeline *pipeline = nullptr;
    QRhiShaderResourceBindings *resourceBindings;
    QVarLengthArray<QRhiShaderStage, 4> shaders;
    QVarLengthArray<QRhiShaderResourceBinding, 8> bindings;
    QVarLengthArray<QRhiVertexInputAttribute, 8> vertexInputs;
    QVarLengthArray<QRhiVertexInputBinding, 8> vertexInputBindings;
    QVarLengthArray<QRhiCommandBuffer::VertexInput, 8> vertexInputBuffers;
    Pipeline::Topology topology = Pipeline::Topology::Triangles;
};

struct BufferBase::Private
{
    QRhiBuffer *buffer;
};

struct TextureBase::Private
{
    QRhiTexture *image;
    QRhiSampler *sampler;
};

struct BindingSet::Private
{
    QRhiShaderResourceBindings *bindings;
    QVarLengthArray<QRhiShaderResourceBinding, 8> resources;
};

struct PlotRenderer::Private
{
    class Node : public QSGRenderNode
    {
    public:
        Node(PlotRenderer *rend)
            : m_renderer(rend)
        {
        }

        QSGRenderNode::RenderingFlags flags() const override
        {
            // By returning NoExternalRendering the scene graph renderer doesn't call beginExternal()
            // endExternal() around the call to render(), which we need given we're not calling directly
            // OpenGL/Vulkan stuff, but we're only going through the QRhi interface
            return QSGRenderNode::DepthAwareRendering | QSGRenderNode::NoExternalRendering;
        }

        void prepare() final
        {
            // matrix() returns a dangling pointer in render(), so copy it here for later.
            // See https://bugreports.qt.io/browse/QTBUG-97589
            m_matrix = *matrix();

            m_renderer->d->prepare();
            m_renderer->d->updateBatch = m_renderer->d->rhi()->nextResourceUpdateBatch();
            m_renderer->prepare();

            m_renderer->d->cmdbuf->resourceUpdate(m_renderer->d->updateBatch);

        }

        void render(const QSGRenderNode::RenderState *state) final
        {
            QMatrix4x4 m = (*state->projectionMatrix()) * m_matrix;

            m_renderer->render(m);

            m_renderer->d->cmdbuf = nullptr;
        }

        PlotRenderer *m_renderer;
        QMatrix4x4 m_matrix;
    };

    QRhi *rhi()
    {
        auto ri = window->rendererInterface();
        return static_cast<QRhi *>(ri->getResource(window, QSGRendererInterface::RhiResource));
    }

    QRhiRenderPassDescriptor *renderPassDescriptor() const
    {
        auto ri = window->rendererInterface();
        auto swapchain = static_cast<QRhiSwapChain *>(ri->getResource(window,
                                                                      QSGRendererInterface::RhiSwapchainResource));
        if (swapchain) {
            return swapchain->renderPassDescriptor();
        }

        auto target = static_cast<QRhiRenderTarget *>(ri->getResource(window,
                                                                      QSGRendererInterface::RhiRedirectRenderTarget));
        return target->renderPassDescriptor();
    }

    std::tuple<QSize, QRhiCommandBuffer *> getRenderResources()
    {
        auto ri = window->rendererInterface();
        auto swapchain = static_cast<QRhiSwapChain *>(ri->getResource(window,
                                                                      QSGRendererInterface::RhiSwapchainResource));

        if (swapchain) {
            return { swapchain->currentPixelSize(), swapchain->currentFrameCommandBuffer() };
        }

        auto cmdbuf = static_cast<QRhiCommandBuffer *>(ri->getResource(window,
                                                                       QSGRendererInterface::RhiRedirectCommandBuffer));
        auto target = static_cast<QRhiRenderTarget *>(ri->getResource(window,
                                                                      QSGRendererInterface::RhiRedirectRenderTarget));

        return { target->pixelSize(), cmdbuf };
    }

    void prepare()
    {
        auto [s, cbuf] = getRenderResources();
        Q_ASSERT(cbuf);
        cmdbuf = cbuf;
        size = s;
    }

    QQuickWindow *window = nullptr;
    Node *node;
    QRectF chartRect;
    float scaleFactor;
    Pipeline::Private *pipeline = nullptr;
    QRhiCommandBuffer *cmdbuf = nullptr;
    QSize size;
    QRhiResourceUpdateBatch *updateBatch;
};

PlotRenderer::PlotRenderer()
            : d(std::make_unique<Private>())
{
    d->node = new Private::Node(this);
}

PlotRenderer::~PlotRenderer()
{
}

QRectF PlotRenderer::rect() const
{
    return d->chartRect;
}

QSGNode *PlotRenderer::sgNode()
{
    return d->node;
}

void PlotRenderer::bindPipeline(const Pipeline &pipeline)
{
    d->cmdbuf->setGraphicsPipeline(pipeline.d->pipeline);
    d->cmdbuf->setVertexInput(0, pipeline.d->vertexInputBuffers.size(), pipeline.d->vertexInputBuffers.data());

    d->cmdbuf->setViewport(QRhiViewport(0, 0, d->size.width(), d->size.height()));

    const int y = d->size.height() - d->chartRect.y() * d->scaleFactor - d->chartRect.height() * d->scaleFactor;
    d->cmdbuf->setScissor(QRhiScissor(d->chartRect.x() * d->scaleFactor, y, d->chartRect.width() * d->scaleFactor, d->chartRect.height() * d->scaleFactor));
}

void PlotRenderer::bindBindingSet(const BindingSet &set)
{
    d->cmdbuf->setShaderResources(set.d->bindings);
}

void PlotRenderer::draw(int count)
{
    d->cmdbuf->draw(count);
}

BindingSet PlotRenderer::createBindingSet()
{
    BindingSet s;
    s.d->bindings = d->rhi()->newShaderResourceBindings();
    return s;
}

BufferBase PlotRenderer::createBufferBase(BufferBase::Type type, BufferBase::UsageFlags usage, uint32_t size)
{
    auto buf = d->rhi()->newBuffer([=]() {
        switch (type) {
            case BufferBase::Type::Immutable: return QRhiBuffer::Type::Immutable;
            case BufferBase::Type::Static: return QRhiBuffer::Type::Static;
            case BufferBase::Type::Dynamic: return QRhiBuffer::Type::Dynamic;
        }
        return QRhiBuffer::Type::Dynamic;
    }(), [=]() {
        QRhiBuffer::UsageFlags f = {};
        if (usage & BufferBase::UsageFlag::VertexBuffer)
            f |= QRhiBuffer::VertexBuffer;
        if (usage & BufferBase::UsageFlag::IndexBuffer)
            f |= QRhiBuffer::IndexBuffer;
        if (usage & BufferBase::UsageFlag::UniformBuffer)
            f |= QRhiBuffer::UniformBuffer;
        if (usage & BufferBase::UsageFlag::StorageBuffer)
            f |= QRhiBuffer::StorageBuffer;
        return f;
    }(), size);
    buf->create();
    BufferBase b;
    b.d->buffer = buf;
    return b;
}

TextureBase PlotRenderer::createTextureBase(TextureFormat f, QSize size)
{
    QRhiTexture::Format format = [=]()
    {
        switch (f) {
            case TextureFormat::RGBA8: return QRhiTexture::Format::RGBA8;
            case TextureFormat::R32F: return QRhiTexture::Format::R32F;
        }
        return QRhiTexture::Format::RGBA8;
    }();

    auto rhi = d->rhi();
    TextureBase tex;
    tex.d->image = rhi->newTexture(format, size);
    tex.d->image->create();

    tex.d->sampler = rhi->newSampler(QRhiSampler::Filter::Linear, QRhiSampler::Filter::Linear, QRhiSampler::Filter::None,
                                     QRhiSampler::AddressMode::ClampToEdge, QRhiSampler::AddressMode::Repeat);
    tex.d->sampler->create();
    return tex;
}

void PlotRenderer::updateTextureBase(TextureBase &tex, const QRect &region, void *data, uint32_t size)
{
    QRhiTextureSubresourceUploadDescription subres(data, size);
    subres.setDataStride(size);
    subres.setDestinationTopLeft({ region.x(), region.y()});
    subres.setSourceTopLeft({ 0, 0 });
    subres.setSourceSize({ region.width() * region.height(), 1 });

    if (!d->updateBatch) {
        d->updateBatch = d->rhi()->nextResourceUpdateBatch();
    }

    d->updateBatch->uploadTexture(tex.d->image, QRhiTextureUploadEntry(0, 0, subres));
}



void PlotRenderer::update(QQuickWindow *window, Plot *plot, const QRect &chartRect, double devicePixelRatio)
{
    d->chartRect = QRectF(chartRect.x(), chartRect.y(),
                          chartRect.width(), chartRect.height());
    d->scaleFactor = devicePixelRatio;
    d->window = window;
}

BufferBase::BufferBase()
          : d(std::make_unique<Private>())
{
}

BufferBase::BufferBase(BufferBase &&b)
          : d(std::move(b.d))
{
}

BufferBase::~BufferBase()
{
}

BufferBase &BufferBase::operator=(BufferBase &&b)
{
    d = std::move(b.d);
    return *this;
}

void BufferBase::update(tl::function_ref<void (char *)> cb)
{
    auto data = d->buffer->beginFullDynamicBufferUpdateForCurrentFrame();
    cb(data);
    d->buffer->endFullDynamicBufferUpdateForCurrentFrame();
}


TextureBase::TextureBase()
           : d(std::make_unique<Private>())
{
}

TextureBase::TextureBase(TextureBase &&) = default;

TextureBase::~TextureBase() = default;

TextureBase &TextureBase::operator=(chart_qt::TextureBase &&) = default;




Pipeline::Pipeline()
        : d(std::make_unique<Private>())
{
}

Pipeline::~Pipeline()
{
}

bool Pipeline::isCreated() const
{
    return d->pipeline;
}

void Pipeline::setTopology(Topology t)
{
    d->topology = t;
}

void Pipeline::setShader(Pipeline::ShaderStage stage, const QString &source)
{
    QFile file(source);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot open shader source file '%s'", qPrintable(source));
        return;
    }

    QShader shader = QShader::fromSerialized(file.readAll());
    auto s = [=]() {
        switch (stage) {
            case ShaderStage::Vertex: return QRhiShaderStage::Type::Vertex;
            case ShaderStage::Fragment: return QRhiShaderStage::Type::Fragment;
        }
        return QRhiShaderStage::Type::Vertex;
    }();
    d->shaders.push_back({ s, shader });
}

static QRhiShaderResourceBinding::StageFlags stageFlags(Pipeline::ShaderStages stages)
{
    QRhiShaderResourceBinding::StageFlags s = QRhiShaderResourceBinding::StageFlag(0);
    if (stages & Pipeline::ShaderStage::Vertex)
        s |= QRhiShaderResourceBinding::StageFlag::VertexStage;
    if (stages & Pipeline::ShaderStage::Fragment)
        s |= QRhiShaderResourceBinding::StageFlag::FragmentStage;
    return s;
}

void Pipeline::addUniformBufferBinding(int binding, Pipeline::ShaderStages stages)
{
    d->bindings.push_back(QRhiShaderResourceBinding::uniformBuffer(binding, stageFlags(stages), nullptr));
}

void Pipeline::addSampledTexture(int binding, Pipeline::ShaderStages stages)
{
    d->bindings.push_back(QRhiShaderResourceBinding::sampledTexture(binding, stageFlags(stages), nullptr, nullptr));
}

void Pipeline::addVertexInput(int binding, int location, VertexInputFormat format, uint32_t offset, uint32_t stride)
{
    auto f = [=]() {
        switch (format) {
            case VertexInputFormat::Float4: return QRhiVertexInputAttribute::Format::Float4;
            case VertexInputFormat::Float3: return QRhiVertexInputAttribute::Format::Float3;
            case VertexInputFormat::Float2: return QRhiVertexInputAttribute::Format::Float2;
            case VertexInputFormat::Float: return QRhiVertexInputAttribute::Format::Float;
            case VertexInputFormat::UNormByte4: return QRhiVertexInputAttribute::Format::UNormByte4;
            case VertexInputFormat::UNormByte2: return QRhiVertexInputAttribute::Format::UNormByte2;
            case VertexInputFormat::UNormByte: return QRhiVertexInputAttribute::Format::UNormByte;
            case VertexInputFormat::UInt4: return QRhiVertexInputAttribute::Format::UInt4;
            case VertexInputFormat::UInt3: return QRhiVertexInputAttribute::Format::UInt3;
            case VertexInputFormat::UInt2: return QRhiVertexInputAttribute::Format::UInt2;
            case VertexInputFormat::UInt: return QRhiVertexInputAttribute::Format::UInt;
            case VertexInputFormat::SInt4: return QRhiVertexInputAttribute::Format::SInt4;
            case VertexInputFormat::SInt3: return QRhiVertexInputAttribute::Format::SInt3;
            case VertexInputFormat::SInt2: return QRhiVertexInputAttribute::Format::SInt2;
            case VertexInputFormat::SInt: return QRhiVertexInputAttribute::Format::SInt;
        }
        return QRhiVertexInputAttribute::Format::Float;
    }();
    d->vertexInputs.push_back({ binding, location, f, offset });
    if (d->vertexInputBindings.size() <= binding) {
        d->vertexInputBindings.resize(binding + 1);
    }
    d->vertexInputBindings[binding] = { stride };
}

void Pipeline::create(PlotRenderer *rend)
{
    auto rhi = rend->d->rhi();
    d->pipeline = rhi->newGraphicsPipeline();

    d->pipeline->setTopology([this]() {
        switch (d->topology) {
            case Topology::Triangles: return QRhiGraphicsPipeline::Topology::Triangles;
            case Topology::TriangleStrip: return QRhiGraphicsPipeline::Topology::TriangleStrip;
            case Topology::TriangleFan: return QRhiGraphicsPipeline::Topology::TriangleFan;
            case Topology::Lines: return QRhiGraphicsPipeline::Topology::Lines;
            case Topology::LineStrip: return QRhiGraphicsPipeline::Topology::LineStrip;
            case Topology::Points: return QRhiGraphicsPipeline::Topology::Points;
        }
        return QRhiGraphicsPipeline::Topology::Triangles;
    }());
    d->pipeline->setRenderPassDescriptor(rend->d->renderPassDescriptor());
    d->pipeline->setFlags(QRhiGraphicsPipeline::Flag::UsesScissor);

    d->pipeline->setShaderStages(d->shaders.begin(), d->shaders.end());

    d->resourceBindings = rhi->newShaderResourceBindings();
    d->resourceBindings->setBindings(d->bindings.begin(), d->bindings.end());
    d->resourceBindings->create();
    d->pipeline->setShaderResourceBindings(d->resourceBindings);

    QRhiVertexInputLayout layout;
    layout.setBindings(d->vertexInputBindings.begin(), d->vertexInputBindings.end());
    layout.setAttributes(d->vertexInputs.begin(), d->vertexInputs.end());
    d->pipeline->setVertexInputLayout(layout);
    d->vertexInputBuffers.resize(d->vertexInputBindings.size());

    d->pipeline->create();
}

void Pipeline::setVertexInputBuffer(int binding, const BufferBase &buffer, uint32_t offset)
{
    d->vertexInputBuffers[binding] = { buffer.d->buffer, offset };
}

BindingSet::BindingSet()
          : d(std::make_unique<Private>())
{
}

BindingSet::BindingSet(BindingSet &&) = default;
BindingSet::~BindingSet() = default;
BindingSet &BindingSet::operator=(BindingSet &&) = default;

void BindingSet::uniformBuffer(int binding, Pipeline::ShaderStages stages, const BufferBase &buffer)
{
    d->resources.push_back(QRhiShaderResourceBinding::uniformBuffer(binding, stageFlags(stages), buffer.d->buffer));
}

void BindingSet::sampledTexture(int binding, Pipeline::ShaderStages stages, const TextureBase &texture)
{
    d->resources.push_back(QRhiShaderResourceBinding::sampledTexture(binding, stageFlags(stages), texture.d->image, texture.d->sampler));
}

void BindingSet::create()
{
    d->bindings->setBindings(d->resources.begin(), d->resources.end());
    d->bindings->create();
}




}
