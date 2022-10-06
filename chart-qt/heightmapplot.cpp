#include "heightmapplot.h"
#include "plot.h"

#include <QFile>
#include <QQuickWindow>
#include <QSGRenderNode>

#include "axis.h"
#include "dataset.h"
#include "renderutils.h"
#include "heightmappipeline.h" // This file was autogenerated

namespace chart_qt {

class HeightmapPlot::Renderer final : public PlotRenderer {
public:
    void init() {
        _pipeline.setTopology(Pipeline::Topology::TriangleStrip);
        _pipeline.create(this);

        _buffer     = createBuffer<HeightmapPipeline::Vertex>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::VertexBuffer, 4);
        _ubuf       = createBuffer<HeightmapPipeline::Ubo>(BufferBase::Type::Dynamic, BufferBase::UsageFlag::UniformBuffer);

        _pipeline.setVertexInputBuffer(_buffer);
    }

    void needsUpdate(DataSet *ds) {
        _dataset = ds;
    }

    void updateData() {
        const int  dataCount = _dataset->getDataCount();
        _data.clear();
        _data.resize(dataCount);

        auto limits = _dataset->dataLimits();
        int width = std::ceil(limits[0].max - limits[0].min);
        int height = std::ceil(limits[1].max - limits[1].min);
        if (!_texture || _texture.pixelSize() != QSize(width, height)) {
            _texture    = createTexture<TextureFormat::R32F>({ width, height});

            _bindingSet = _pipeline.createBindingSet(this, { .ubuf      = _ubuf,
                                                             .tex = _texture });
        }

        const auto xdata = _dataset->getValues(0).data();
        const auto ydata     = _dataset->getValues(1).data();
        const auto zdata     = _dataset->getValues(2).data();

        // for (int i = 0; i < dataCount; ++i) {
        //     float x = xdata[i];
        //     float y = ydata[i];
        //     float z = zdata[i];
        //     _data[x * height + y] = z;
        // }

        updateTexture(_texture, QRect(0, 0, width, height), zdata);

        float startU = _xaxis[0] / width;
        float endU   = _xaxis[1] / width;
        float startV = _yaxis[0] / height;
        float endV   = _yaxis[1] / height;

        _buffer.update([=](auto *data) {
            static const QVector2D vertices[] = {
                { 0, 0 },
                { 0, 1 },
                { 1, 0 },
                { 1, 1 },
            };
            for (int i = 0; i < 4; ++i) {
                data[i].vertex = vertices[i];
            }

            data[0].uv_in = { startU, endV };
            data[1].uv_in = { startU, startV };
            data[2].uv_in = { endU, endV };
            data[3].uv_in = { endU, startV };
        });

        _dataset = nullptr;
    }

    void prepare() final {
        if (!_pipeline.isCreated()) {
            init();
        }
        if (_dataset) {
            updateData();
        }
    }

    void render(const QMatrix4x4 &matrix) final {
        if (_texture) {
            QMatrix4x4 m = matrix;
            m.scale(rect().width(), rect().height());

            _ubuf.update([&](HeightmapPipeline::Ubo *data) {
                memcpy(data->qt_Matrix.data(), m.data(), 64);
                data->gradient   = _gradient;
            });

            bindPipeline(_pipeline);
            bindBindingSet(_bindingSet);
            draw(4);
        }
    }

    void setGradient(float start, float stop) {
        _gradient = { start, stop };
    }

    HeightmapPipeline                 _pipeline;
    Buffer<HeightmapPipeline::Vertex> _buffer;
    Buffer<HeightmapPipeline::Ubo>    _ubuf;
    Texture<TextureFormat::R32F>      _texture;
    BindingSet                        _bindingSet;

    DataSet                          *_dataset    = nullptr;
    QVector2D                         _gradient   = { 0, 1 };
    float                             _xaxis[2]   = { 0, 1 };
    float                             _yaxis[2]   = { 0, 1 };
    float                             _dataWidth  = 1;
    std::vector<float>                _data;
};

HeightmapPlot::HeightmapPlot() {
    _renderer = new Renderer;
    _gradientStart = -1;
    _gradientStop = 1;
}

PlotRenderer *HeightmapPlot::renderer() {
    return _renderer;
}

void HeightmapPlot::update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused) {
    if (auto xa = xAxis()) {
        _renderer->_xaxis[0] = xa->min();
        _renderer->_xaxis[1] = xa->max();
    }

    if (auto ya = yAxis()) {
        _renderer->_yaxis[0] = ya->min();
        _renderer->_yaxis[1] = ya->max();
    }
    _renderer->setGradient(_gradientStart, _gradientStop);
    if (needsUpdate() && !paused) {
        resetNeedsUpdate();

        _renderer->_dataset = dataSet();
    }
}

double HeightmapPlot::gradientStart() const {
    return _gradientStart;
}

void HeightmapPlot::setGradientStart(double g) {
    if (_gradientStart != g) {
        _gradientStart = g;
        emit gradientChanged();
    }
}

double HeightmapPlot::gradientStop() const {
    return _gradientStop;
}

void HeightmapPlot::setGradientStop(double g) {
    if (_gradientStop != g) {
        _gradientStop = g;
        emit gradientChanged();
    }
}

} // namespace chart_qt