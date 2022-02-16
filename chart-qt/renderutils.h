#ifndef CHARTQT_RENDERUTILS_H
#define CHARTQT_RENDERUTILS_H

#include <memory>
#include <span>

#include <tl/function_ref.hpp>

#include <QString>
#include <QSize>
#include <QRect>

class QQuickWindow;
class QRectF;
class QSGNode;
class QMatrix4x4;

namespace chart_qt {

class Plot;
class Pipeline;
class PlotRenderer;
class BindingSet;

class BufferBase
{
public:
    enum class Type {
        Immutable,
        Static,
        Dynamic
    };

    enum class UsageFlag {
        VertexBuffer = 1 << 0,
        IndexBuffer = 1 << 1,
        UniformBuffer = 1 << 2,
        StorageBuffer = 1 << 3
    };
    Q_DECLARE_FLAGS(UsageFlags, UsageFlag)

    BufferBase();
    BufferBase(const BufferBase &) = delete;
    BufferBase(BufferBase &&);
    ~BufferBase();

    BufferBase &operator=(const BufferBase &) = delete;
    BufferBase &operator=(BufferBase &&);

    void update(tl::function_ref<void (char *)> cb);

private:
    struct Private;
    std::unique_ptr<Private> d;

    friend Pipeline;
    friend PlotRenderer;
    friend BindingSet;
};

template<typename... Ts>
struct DataLayout
{
};

template<typename T>
class Buffer : public BufferBase
{
public:
    using BufferBase::BufferBase;
    Buffer(BufferBase &&b) : BufferBase(std::move(b)) {}
    Buffer(Buffer<T> &&);
    ~Buffer() { this->~BufferBase(); }

    using BufferBase::operator=;
    Buffer<T> &operator=(Buffer<T> &&b)
    {
        BufferBase::operator=(std::move(b));
        return *this;
    }

    void update(tl::function_ref<void (T *)> cb)
    {
        BufferBase::update([=](char *data) {
            cb(reinterpret_cast<T *>(data));
        });
    }
};

template<typename T>
class BufferRef
{
public:
    BufferRef(const Buffer<T> &buf) : ref(buf) {}

    template<typename D> requires std::is_same_v<T, typename D::Layout>
    BufferRef(const Buffer<D> &buf) : ref(buf) {}

    const BufferBase &ref;
};

enum class TextureFormat
{
    RGBA8,
    R32F,
};

class TextureBase
{
public:
    TextureBase();
    ~TextureBase();
    TextureBase(const TextureBase &) = delete;
    TextureBase(TextureBase &&);

    TextureBase &operator=(const TextureBase &) = delete;
    TextureBase &operator=(TextureBase &&);

private:
    struct Private;
    std::unique_ptr<Private> d;
    friend class BindingSet;
    friend class PlotRenderer;
};

template<TextureFormat Format>
class Texture : public TextureBase
{
public:
    Texture() {};
    Texture(TextureBase &&t) : TextureBase(std::move(t)) {}
    ~Texture() { this->~TextureBase(); }

    Texture<Format> &operator=(Texture<Format> &&t)
    {
        TextureBase::operator=(std::move(t));
        return *this;
    }
};

class Pipeline
{
public:
    enum class ShaderStage
    {
        Vertex = 1,
        Fragment = 2,
    };
    Q_DECLARE_FLAGS(ShaderStages, ShaderStage);

    enum class VertexInputFormat
    {
        Float4,
        Float3,
        Float2,
        Float,
        UNormByte4,
        UNormByte2,
        UNormByte,
        UInt4,
        UInt3,
        UInt2,
        UInt,
        SInt4,
        SInt3,
        SInt2,
        SInt
    };
    enum class Topology
    {
        Triangles,
        TriangleStrip,
        TriangleFan,
        Lines,
        LineStrip,
        Points
    };

    Pipeline();
    Pipeline(const Pipeline &) = delete;
    Pipeline(Pipeline &&) = default;
    ~Pipeline();

    bool isCreated() const;

    void setTopology(Topology t);
    void setShader(ShaderStage stage, const QString &source);
    void addUniformBufferBinding(int binding, ShaderStages stages);
    void addSampledTexture(int binding, ShaderStages stages);
    void addVertexInput(int binding, int location, VertexInputFormat format, uint32_t offset, uint32_t stride);

    void create(PlotRenderer *renderer);

    void setVertexInputBuffer(int binding, const BufferBase &buffer, uint32_t offset);

private:
    struct Private;
    std::unique_ptr<Private> d;

    friend class PlotRenderer;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Pipeline::ShaderStages);

class BindingSet
{
public:
    BindingSet();
    BindingSet(const BindingSet &) = delete;
    BindingSet(BindingSet &&);
    ~BindingSet();

    void uniformBuffer(int binding, Pipeline::ShaderStages stages, const BufferBase &buffer);
    void sampledTexture(int binding, Pipeline::ShaderStages stages, const TextureBase &texture);

    BindingSet &operator=(const BindingSet &) = delete;
    BindingSet &operator=(BindingSet &&);

    void create();

private:
    struct Private;
    std::unique_ptr<Private> d;

    friend PlotRenderer;
};

namespace {
    template<TextureFormat F> int textureBpp = 4;
};

class PlotRenderer
{
public:
    PlotRenderer();
    virtual ~PlotRenderer();

    virtual void prepare() {}
    virtual void render(const QMatrix4x4 &matrix) = 0;

    QRectF rect() const;

    QSGNode *sgNode();

    void update(QQuickWindow *window, Plot *plot, const QRect &chartRect, double devicePixelRatio);

    void bindPipeline(const Pipeline &pipeline);
    template<typename T>
    void bindPipeline(const T &pipeline) { bindPipeline(pipeline.pipeline()); }
    void bindBindingSet(const BindingSet &set);
    void draw(int count);

    template<typename T>
    Buffer<T> createBuffer(BufferBase::Type type, BufferBase::UsageFlags usage, uint32_t size = 1)
    {
        return createBufferBase(type, usage, size * sizeof(T));
    }

    template<TextureFormat F>
    Texture<F> createTexture(QSize size)
    {
        return createTextureBase(F, size);
    }

    BindingSet createBindingSet();

    template<TextureFormat F>
    void updateTexture(Texture<F> &tex, const QRect &region, void *data)
    {
        updateTextureBase(tex, region, data, region.width() * region.height() * textureBpp<F>);
    }

private:
    BufferBase createBufferBase(BufferBase::Type type, BufferBase::UsageFlags usage, uint32_t size);
    TextureBase createTextureBase(TextureFormat f, QSize size);
    void updateTextureBase(TextureBase &tex, const QRect &region, void *data, uint32_t size);

    struct Private;
    std::unique_ptr<Private> d;

    friend class Pipeline;
};

}

#endif
