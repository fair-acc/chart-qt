#include "chartitem.h"

#include <QLineF>
#include <QQuickWindow>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGRectangleNode>
#include <QSGRenderNode>
#include <QSGTransformNode>

#include "axis.h"
#include "plot.h"
#include "renderutils.h"
#include "xyplot.h"

namespace chart_qt {

struct ChartItem::AxisLayout {
    explicit AxisLayout(ChartItem *chart, Axis *a)
        : chartItem(chart)
        , axis(a) {}

    bool isHorizontal() const {
        const auto pos = axis->position();
        return pos == Axis::Position::Top || pos == Axis::Position::Bottom;
    }

    QQuickItem *getLabel(int idx) {
        if (labels.size() > idx) {
            return labels[idx];
        }
        while (labels.size() <= idx) {
            auto l = axis->createLabel();
            l->setParentItem(chartItem);
            labels.push_back(l);
        }
        return labels[idx];
    };

    void setLabelValue(QQuickItem *label, double value) {
        label->setProperty("text", QString::number(value, 'f', 2));
    }

    ChartItem                *chartItem;
    Axis                     *axis;
    AxisNode                 *node = nullptr;
    QRectF                    rect;
    std::vector<QQuickItem *> labels;
    QLineF                    axisLine;
    std::vector<QLineF>       majorLines;
    std::vector<QLineF>       minorLines;
    bool                      needsUpdate = false;
    bool                      dirty       = true;
};

class ChartItem::AxisNode final : public QSGGeometryNode {
public:
    class Shader : public QSGMaterialShader {
    public:
        Shader() {
            setShaderFileName(VertexStage, QLatin1String(":/shaders/axismaterial.vert.qsb"));
            setShaderFileName(FragmentStage, QLatin1String(":/shaders/axismaterial.frag.qsb"));
        }

        bool updateUniformData(RenderState &state, QSGMaterial *, QSGMaterial *) {
            bool        changed = false;
            QByteArray *buf     = state.uniformData();
            if (state.isMatrixDirty()) {
                const QMatrix4x4 m = state.combinedMatrix();
                memcpy(buf->data(), m.constData(), 64);
                changed = true;
            }
            return changed;
        }
    };

    class Material final : public QSGMaterial {
    public:
        Material() {
        }
        QSGMaterialType *type() const final {
            static QSGMaterialType type;
            return &type;
        }
        QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override {
            return new Shader;
        }
    };

    AxisNode(ChartItem::AxisLayout *layout)
        : _layout(layout) {
        auto geo = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
        geo->setDrawingMode(QSGGeometry::DrawLines);
        geo->setVertexDataPattern(QSGGeometry::DynamicPattern);
        setGeometry(geo);

        setMaterial(new Material);

        setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
    }

    void update() {
        auto geo = geometry();

        if (_layout->needsUpdate) {
            const int numPoints = (1 + _layout->majorLines.size() + _layout->minorLines.size()) * 2;
            if (geo->vertexCount() != numPoints) {
                geo->allocate(numPoints);
            }

            auto data = geo->vertexDataAsColoredPoint2D();
            data[0].set(_layout->axisLine.x1(), _layout->axisLine.y1(), 0, 0, 0, 0xff);
            data[1].set(_layout->axisLine.x2(), _layout->axisLine.y2(), 0, 0, 0, 0xff);

            data += 2;
            for (int i = 0; i < _layout->majorLines.size(); ++i) {
                const auto &l = _layout->majorLines[i];
                data[0].set(l.x1(), l.y1(), 0xbb, 0xbb, 0xbb, 0xff);
                data[1].set(l.x2(), l.y2(), 0xbb, 0xbb, 0xbb, 0xff);
                data += 2;
            }

            for (int i = 0; i < _layout->minorLines.size(); ++i) {
                const auto &l = _layout->minorLines[i];
                data[0].set(l.x1(), l.y1(), 0x80, 0x80, 0x80, 0xff);
                data[1].set(l.x2(), l.y2(), 0x80, 0x80, 0x80, 0xff);
                data += 2;
            }

            _layout->needsUpdate = false;
            geo->markVertexDataDirty();

            markDirty(QSGNode::DirtyGeometry);
        }
    }

    ChartItem::AxisLayout *_layout;
};

ChartItem::ChartItem(QQuickItem *parent)
    : QQuickItem(parent) {
    setFlag(QQuickItem::ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptTouchEvents(true);
}

ChartItem::~ChartItem() {
}

void ChartItem::addPlot(Plot *plot) {
    _plotsToInit.push_back(plot);
    _plots.push_back(plot);

    connect(plot, &Plot::updateNeeded, this, &QQuickItem::update);

    update();
}

void ChartItem::addAxis(Axis *axis) {
    _axes.push_back(std::make_unique<AxisLayout>(this, axis));
    _addedAxes.push_back(axis);
    auto a = _axes.back().get();
    connect(axis, &Axis::minChanged, this, [this, a]() {
        a->dirty = true;
        polish();
    });
    connect(axis, &Axis::maxChanged, this, [this, a]() {
        a->dirty = true;
        polish();
    });
    polish();
}

bool ChartItem::paused() const {
    return _paused;
}

void ChartItem::setPaused(bool p) {
    if (_paused != p) {
        _paused = p;
        emit pausedChanged();
    }
}

const std::vector<Axis *> &ChartItem::axes() const {
    return _addedAxes;
}

void ChartItem::componentComplete() {
    QQuickItem::componentComplete();
    polish();
}

void ChartItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    const bool wchanged = newGeometry.width() != oldGeometry.width();
    const bool hchanged = newGeometry.height() != oldGeometry.height();
    for (auto &a : _axes) {
        bool h = a->isHorizontal();
        if ((h && wchanged) || (!h && hchanged)) {
            a->dirty = true;
        }
    }

    polish();
    update();
}

void ChartItem::updateAxesRect() {
    int  leftMargin   = 0;
    int  rightMargin  = 0;
    int  topMargin    = 0;
    int  bottomMargin = 0;

    bool crectChanged = false;
    for (auto &a : _axes) {
        double w;
        if (a->dirty) {
            crectChanged = true;
            a->dirty     = false;

            for (auto l : a->labels) {
                l->setVisible(false);
            }

            const bool horiz = a->isHorizontal();

            auto       l     = a->getLabel(0);
            a->setLabelValue(l, a->axis->min());
            w = horiz ? l->implicitHeight() : l->implicitWidth();
            a->setLabelValue(l, a->axis->max());
            w = std::max(w, horiz ? l->implicitHeight() : l->implicitWidth());
            w += 20;
        } else {
            w = [&]() {
                switch (a->axis->position()) {
                case Axis::Position::Left:
                case Axis::Position::Right: return a->rect.width();
                case Axis::Position::Top:
                case Axis::Position::Bottom: return a->rect.height();
                }
                return 0.;
            }();
        }

        switch (a->axis->position()) {
        case Axis::Position::Left:
            a->rect = QRectF(leftMargin, 0, w, height());
            leftMargin += w;
            break;
        case Axis::Position::Right:
            a->rect = QRectF(width() - rightMargin - w, 0, w, height());
            rightMargin += w;
            break;
        case Axis::Position::Top:
            a->rect = QRectF(0, topMargin, width(), w);
            topMargin += w;
            break;
        case Axis::Position::Bottom:
            a->rect = QRectF(0, height() - bottomMargin - w, width(), w);
            bottomMargin += w;
            break;
        }
    }
    if (crectChanged) {
        emit implicitContentRectChanged();
    }
}

void ChartItem::updatePolish() {
    updateAxesRect();
    for (auto &a : _axes) {
        const auto findSubdivision = [](double range) {
            const double multiplier = pow(10, floor(log10(range)));
            double       firstDigit = trunc(range / multiplier);
            if (firstDigit > 5) {
                firstDigit = 10;
            } else if (firstDigit > 2) {
                firstDigit = 5;
            }
            return (firstDigit * multiplier) * 100;
        };

        const double min                       = a->axis->min();
        const double max                       = a->axis->max();
        const double range                     = max - min;

        const auto   axisPos                   = a->axis->position();
        const bool   horiz                     = axisPos == Axis::Position::Top || axisPos == Axis::Position::Bottom;
        const bool   inverted                  = a->axis->isRightToLeftOrBottomToTop();

        auto         rect                      = contentRect();

        const double pixelSize                 = horiz ? rect.width() : rect.height();

        const double majorLinesLogicalDistance = findSubdivision(range / pixelSize);
        const double minorLinesLogicalDistance = majorLinesLogicalDistance / 10.;
        const double logicalToPixel            = pixelSize / range;
        const double majorLinesPixelDistance   = (inverted ? -1. : 1.) * majorLinesLogicalDistance * logicalToPixel;
        const double minorLinesPixelDistance   = (inverted ? -1. : 1.) * minorLinesLogicalDistance * logicalToPixel;

        a->majorLines.clear();
        a->minorLines.clear();

        int       labelIndex      = 0;
        const int tickLength      = (axisPos == Axis::Position::Bottom || axisPos == Axis::Position::Right) ? 10 : -10;
        const int minorTickLength = float(tickLength) * 0.6;

        // round to int to get a sharper line
        const double basePos     = std::round(axisPos == Axis::Position::Bottom ? a->rect.top() : (axisPos == Axis::Position::Right ? a->rect.left() : (horiz ? a->rect.bottom() : a->rect.right())));
        const double linesOffset = (horiz ? rect.left() : rect.top());

        if (horiz) {
            a->axisLine = { std::round(rect.left()), basePos, std::round(rect.right()), basePos };
        } else {
            a->axisLine = { basePos, std::round(rect.top()), basePos, std::round(rect.bottom()) };
        }

        auto placeLabel = [&](QQuickItem *label, double linePos) {
            if (horiz) {
                label->setX(std::round(linePos - label->implicitWidth() / 2.));
                label->setY(std::round(basePos + tickLength - (axisPos == Axis::Position::Top ? label->implicitHeight() : 0)));
            } else {
                label->setY(std::round(linePos - label->implicitHeight() / 2));
                label->setX(std::round(basePos + tickLength - (axisPos == Axis::Position::Left ? label->implicitWidth() : 0)));
            }
            label->setVisible(true);
        };

        // 'min' is not actually the minimum visible value, due to the margins
        const double visibleMin     = min - linesOffset / logicalToPixel;
        const double endPos         = (horiz ? rect.right() : rect.bottom());
        const double startPos       = horiz ? rect.left() : rect.top();

        double       startLineValue = visibleMin - fmod(visibleMin, majorLinesLogicalDistance) - majorLinesLogicalDistance;
        double       startLinePos   = (startLineValue - min) * logicalToPixel + linesOffset;
        if (inverted) {
            startLinePos = (horiz ? width() : rect.bottom()) - startLinePos;
        }
        const auto keepGoing = [&](double p) { return inverted ? p >= startPos : p <= endPos; };
        for (int majorLine = 0;; ++majorLine) {
            double linePos   = startLinePos + majorLinesPixelDistance * majorLine;
            double lineValue = startLineValue + majorLinesLogicalDistance * majorLine;

            if (!keepGoing(linePos)) {
                break;
            }

            if (inverted ? linePos <= endPos : linePos >= startPos) {
                auto label = a->getLabel(++labelIndex);
                a->setLabelValue(label, lineValue);
                placeLabel(label, linePos);

                const double pos = std::round(linePos);
                if (horiz) {
                    a->majorLines.push_back(QLineF{ pos, std::round(axisPos == Axis::Position::Bottom ? rect.top() : rect.bottom()),
                            pos, basePos });
                } else {
                    a->majorLines.push_back(QLineF{ std::round(axisPos == Axis::Position::Right ? rect.left() : rect.right()), pos,
                            basePos, pos });
                }
            }

            for (int minorLine = 0; minorLine < 10; ++minorLine) {
                double       minorLineValue = lineValue + minorLinesLogicalDistance * minorLine;
                double       minorLinePos   = linePos + minorLinesPixelDistance * minorLine;

                const double pos            = std::round(minorLinePos);
                if (inverted ? pos <= endPos : pos >= startPos) {
                    auto tl = (minorLine == 5 || minorLine == 0) ? tickLength : minorTickLength;
                    if (horiz) {
                        a->minorLines.push_back(QLineF{ pos, basePos, pos, basePos + tl });
                    } else {
                        a->minorLines.push_back(QLineF{ basePos, pos, basePos + tl, pos });
                    }

                    if (minorLine == 5) {
                        auto label = a->getLabel(++labelIndex);
                        a->setLabelValue(label, minorLineValue);

                        // check if the label would fit, otherwise don't show it
                        const double neededSpace = horiz ? label->implicitWidth() : label->implicitHeight();
                        if (fabs(majorLinesPixelDistance) > neededSpace * 2.5) {
                            placeLabel(label, minorLinePos);
                        } else {
                            --labelIndex;
                        }
                    }
                }
            }
        }
        a->needsUpdate = true;
    }
}

QSGNode *ChartItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *) {
    if (!node) {
        node = new QSGNode;
        node->appendChildNode(new QSGTransformNode);
        node->appendChildNode(window()->createRectangleNode());
    }

    for (auto &a : _axes) {
        if (!a->node) {
            a->node = new AxisNode(a.get());
            node->insertChildNodeAfter(a->node, node->firstChild());
        }
        a->node->update();
    }

    auto       crect = contentRect();

    QMatrix4x4 matrix;
    matrix.translate(crect.left(), crect.top());
    auto plotsParentNode = node->firstChild();
    static_cast<QSGTransformNode *>(plotsParentNode)->setMatrix(matrix);

    for (auto p : _plotsToInit) {
        plotsParentNode->appendChildNode(p->renderer()->sgNode());
    }
    _plotsToInit.clear();

    auto rect = mapRectToScene(crect).toRect();

    for (auto p : _plots) {
        p->update(window(), rect, window()->effectiveDevicePixelRatio(), false);
        p->renderer()->update(window(), p, rect, window()->effectiveDevicePixelRatio());
    }
    return node;
}

QRectF ChartItem::contentRect() const {
    auto crect = implicitContentRect();
    if (!_minimumMargins.isNull()) {
        auto r = QRectF(0, 0, width(), height()).marginsRemoved(_minimumMargins);
        return crect.intersected(r);
    }
    return crect;
}

QRectF ChartItem::implicitContentRect() const {
    int leftMargin   = 0;
    int rightMargin  = 0;
    int topMargin    = 0;
    int bottomMargin = 0;
    for (auto &a : _axes) {
        switch (a->axis->position()) {
        case Axis::Position::Left:
            leftMargin += a->rect.width();
            break;
        case Axis::Position::Right:
            rightMargin += a->rect.width();
            break;
        case Axis::Position::Top:
            topMargin += a->rect.height();
            break;
        case Axis::Position::Bottom:
            bottomMargin += a->rect.height();
            break;
        }
    }
    return QRectF(leftMargin, topMargin, width() - leftMargin - rightMargin, height() - topMargin - bottomMargin);
}

QRectF ChartItem::axisRect(Axis *axis) const {
    for (auto &a : _axes) {
        if (a->axis == axis) {
            return a->rect;
        }
    }
    return {};
}

void ChartItem::zoomIn(QRectF area) {
    area = sanitizeZoomRect(area);
    if (area.isEmpty()) {
        return;
    }

    for (auto &a : _axes) {
        const auto   range         = a->axis->max() - a->axis->min();
        const bool   horiz         = a->isHorizontal();

        const double fullPixelSize = horiz ? (width() - 2. * _verticalMargin) : (height() - 2. * _horizontalMargin);

        double       min           = horiz ? area.x() - _verticalMargin : area.y() - _horizontalMargin;
        double       max           = horiz ? area.right() - _verticalMargin : area.bottom() - _horizontalMargin;
        if (a->axis->isRightToLeftOrBottomToTop()) {
            min = horiz ? width() - _verticalMargin - area.right() : height() - _horizontalMargin - area.bottom();
            max = horiz ? width() - _verticalMargin - area.x() : height() - _horizontalMargin - area.y();
        }

        a->axis->setMax(a->axis->min() + range * max / fullPixelSize);
        a->axis->setMin(a->axis->min() + range * min / fullPixelSize);
    }

    _zoomHistory.push(area);
}

void ChartItem::setMinimumContentMargins(const QMarginsF &margins) {
    _minimumMargins = margins;
    polish();
}

void ChartItem::zoomOut(QRectF area) {
    area = sanitizeZoomRect(area);
    if (area.isEmpty()) {
        return;
    }

    for (auto &a : _axes) {
        const auto   range         = a->axis->max() - a->axis->min();
        const bool   horiz         = a->isHorizontal();
        const double fullPixelSize = horiz ? area.width() : area.height();

        double       min           = horiz ? -area.x() + _verticalMargin : -area.y() + _horizontalMargin;
        double       max           = horiz ? width() - _verticalMargin - area.x() : height() - 2. * _horizontalMargin - area.y();
        if (a->axis->isRightToLeftOrBottomToTop()) {
            min = horiz ? _horizontalMargin - (width() - area.right()) : _horizontalMargin - (height() - area.bottom());
            max = horiz ? 0 : area.bottom() - _horizontalMargin;
        }

        a->axis->setMax(a->axis->min() + range * max / fullPixelSize);
        a->axis->setMin(a->axis->min() + range * min / fullPixelSize);
    }
}

void ChartItem::undoZoom() {
    if (!_zoomHistory.empty()) {
        zoomOut(_zoomHistory.top());
        _zoomHistory.pop();
    }
}

QRectF ChartItem::sanitizeZoomRect(QRectF rect) {
    if (rect.right() < rect.x()) {
        double x = rect.right();
        rect.setRight(rect.x());
        rect.setX(x);
    }
    if (rect.bottom() < rect.y()) {
        double y = rect.bottom();
        rect.setBottom(rect.y());
        rect.setY(y);
    }
    return rect;
}

} // namespace chart_qt
