#include "chartitem.h"

#include <QSGRenderNode>
#include <QSGTransformNode>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGRectangleNode>
#include <QLineF>
#include <QQuickWindow>

#include "plot.h"
#include "sindataset.h"
#include "xyplot.h"
#include "axis.h"

namespace chart_qt {

struct ChartItem::AxisLayout
{
    explicit AxisLayout(Axis *a)
        : axis(a)
    {}

    bool isInverted() const
    {
        const auto pos = axis->position();
        const auto dir = axis->direction();
        const bool horiz = pos == Axis::Position::Top || pos == Axis::Position::Bottom;
        return (horiz && dir == Axis::Direction::RightToLeft) ||
               (!horiz && dir == Axis::Direction::BottomToTop);
    }

    inline void zoom(double m)
    {
        axis->setMax((axis->max() - axis->min()) / m + axis->min());
    }

    Axis *axis;
    AxisNode *node = nullptr;
    std::vector<QQuickItem *> labels;
    QLineF axisLine;
    std::vector<QLineF> majorLines;
    std::vector<QLineF> minorLines;
    bool needsUpdate = false;
};

class ChartItem::AxisNode final : public QSGGeometryNode
{
public:
    class Shader : public QSGMaterialShader
    {
    public:
        Shader()
        {
            setShaderFileName(VertexStage, QLatin1String(":/src/shaders/axismaterial.vert.qsb"));
            setShaderFileName(FragmentStage, QLatin1String(":/src/shaders/axismaterial.frag.qsb"));
        }

        bool updateUniformData(RenderState &state, QSGMaterial *, QSGMaterial *)
        {
            bool changed = false;
            QByteArray *buf = state.uniformData();
            if (state.isMatrixDirty()) {
                const QMatrix4x4 m = state.combinedMatrix();
                memcpy(buf->data(), m.constData(), 64);
                changed = true;
            }
            return changed;
        }
    };


    class Material final : public QSGMaterial
    {
    public:
        Material()
        {

        }
        QSGMaterialType *type() const final
        {
            static QSGMaterialType type;
            return &type;
        }
        QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override
        {
            return new Shader;
        }
    };

    AxisNode(ChartItem::AxisLayout *layout)
        : m_layout(layout)
    {
        auto geo = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0);
        geo->setDrawingMode(QSGGeometry::DrawLines);
        geo->setVertexDataPattern(QSGGeometry::DynamicPattern);
        setGeometry(geo);

        setMaterial(new Material);

        setFlags(QSGNode::OwnsGeometry | QSGNode::OwnsMaterial);
    }

    void update()
    {
        auto geo = geometry();

        if (m_layout->needsUpdate) {
            const int numPoints = (1 + m_layout->majorLines.size() + m_layout->minorLines.size()) * 2;
            if (geo->vertexCount() != numPoints) {
                geo->allocate(numPoints);
            }

            auto data = geo->vertexDataAsColoredPoint2D();
            data[0].set(m_layout->axisLine.x1(), m_layout->axisLine.y1(), 0, 0, 0, 0xff);
            data[1].set(m_layout->axisLine.x2(), m_layout->axisLine.y2(), 0, 0, 0, 0xff);

            data += 2;
            for (int i = 0; i < m_layout->majorLines.size(); ++i) {
                const auto &l = m_layout->majorLines[i];
                data[0].set(l.x1(), l.y1(), 0xbb, 0xbb, 0xbb, 0xff);
                data[1].set(l.x2(), l.y2(), 0xbb, 0xbb, 0xbb, 0xff);
                data += 2;
            }

            for (int i = 0; i < m_layout->minorLines.size(); ++i) {
                const auto &l = m_layout->minorLines[i];
                data[0].set(l.x1(), l.y1(), 0x80, 0x80, 0x80, 0xff);
                data[1].set(l.x2(), l.y2(), 0x80, 0x80, 0x80, 0xff);
                data += 2;
            }

            m_layout->needsUpdate = false;
            geo->markVertexDataDirty();

            markDirty(QSGNode::DirtyGeometry);
        }
    }

    ChartItem::AxisLayout *m_layout;
};

ChartItem::ChartItem(QQuickItem *parent)
         : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptTouchEvents(true);

    // auto dataset = new SinDataSet;
    // auto plot = new XYPlot;
    // plot->setDataSet(dataset);
    // addPlot(plot);
}

static SinDataSet *dataset()
{
    static SinDataSet ds;
    return &ds;
}

ChartItem::~ChartItem()
{

}

void ChartItem::addPlot(Plot *plot)
{
    plot->setDataSet(dataset());

    m_plotsToInit.push_back(plot);
    m_plots.push_back(plot);

    connect(plot, &Plot::updateNeeded, this, &QQuickItem::update);

    update();
}

void ChartItem::addAxis(Axis *axis)
{
    m_axes.push_back(std::make_unique<AxisLayout>(axis));
    connect(axis, &Axis::minChanged, this, &QQuickItem::polish);
    connect(axis, &Axis::maxChanged, this, &QQuickItem::polish);
}

bool ChartItem::paused() const
{
    return m_paused;
}

void ChartItem::setPaused(bool p)
{
    if (m_paused != p) {
        m_paused = p;
        emit pausedChanged();
    }
}

QColor ChartItem::zoomRectColor() const
{
    return m_zoomRectColor;
}

void ChartItem::setZoomRectColor(const QColor &c)
{
    if (m_zoomRectColor != c) {
        m_zoomRectColor = c;
        emit zoomRectColorChanged();
        update();
    }
}

void ChartItem::componentComplete()
{
    QQuickItem::componentComplete();
    polish();
}

void ChartItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    polish();
    update();
}

void ChartItem::updatePolish()
{
    for (auto &a: m_axes) {
        auto getLabel = [&](int idx) {
            if (a->labels.size() > idx) {
                return a->labels[idx];
            }
            while (a->labels.size() <= idx) {
                auto l = a->axis->createLabel();
                l->setParentItem(this);
                a->labels.push_back(l);
            }
            return a->labels[idx];
        };

        for (auto l: a->labels) {
            l->setVisible(false);
        }

        const auto findSubdivision = [](double range) {
            const double multiplier = pow(10, floor(log10(range)));
            double firstDigit = trunc(range / multiplier);
            if (firstDigit > 5) {
                firstDigit = 10;
            } else if (firstDigit > 2) {
                firstDigit = 5;
            }
            return (firstDigit * multiplier) * 100;
        };

        const double min = a->axis->min();
        const double max = a->axis->max();
        const double range = max - min;

        const auto axisPos = a->axis->position();
        const bool horiz = axisPos == Axis::Position::Top || axisPos == Axis::Position::Bottom;
        const bool inverted = a->isInverted();

        const double pixelSize = horiz ? width() - 2. * m_verticalMargin : height() - 2. * m_horizontalMargin;

        const double majorLinesLogicalDistance = findSubdivision(range / pixelSize);
        const double minorLinesLogicalDistance = majorLinesLogicalDistance / 10.;
        const double logicalToPixel = pixelSize / range;
        const double majorLinesPixelDistance = (inverted ? -1. : 1.) * majorLinesLogicalDistance * logicalToPixel;
        const double minorLinesPixelDistance = (inverted ? -1. : 1.) * minorLinesLogicalDistance * logicalToPixel;

        a->majorLines.clear();
        a->minorLines.clear();

        int labelIndex = 0;
        const int tickLength = (axisPos == Axis::Position::Bottom || axisPos == Axis::Position::Right) ? 10 : -10;

        // round to int to get a sharper line
        const double basePos = axisPos == Axis::Position::Bottom ? qRound(height() - m_horizontalMargin) :
                               (axisPos == Axis::Position::Right ? qRound(width() - m_verticalMargin) :
                               (horiz ? m_horizontalMargin : m_verticalMargin));
        const double linesOffset = (horiz ? m_verticalMargin : m_horizontalMargin);

        if (horiz) {
            a->axisLine = { 0., basePos, width(), basePos };
        } else {
            a->axisLine = { basePos, 0., basePos, height() };
        }

        auto placeLabel = [&](QQuickItem *label, double linePos) {
            if (horiz) {
                label->setX(qRound(linePos - label->implicitWidth() / 2.));
                label->setY(basePos + tickLength - (axisPos == Axis::Position::Top ? label->implicitHeight() : 0));
            } else {
                label->setY(qRound(linePos - label->implicitHeight() / 2));
                label->setX(basePos + tickLength - (axisPos == Axis::Position::Left ? label->implicitWidth() : 0));
            }
            label->setVisible(true);
        };

        // 'min' is not actually the minimum visible value, due to the margins
        const double visibleMin = min - linesOffset / logicalToPixel;
        const double endPos = (horiz ? width() : height());
        double lineValue = visibleMin - fmod(visibleMin, majorLinesLogicalDistance) - majorLinesLogicalDistance;
        double linePos = (lineValue - min) * logicalToPixel + linesOffset;
        if (inverted) {
            linePos = (horiz ? width() : height()) - linePos;
        }
        const auto keepGoing = [&]() { return inverted ? linePos >= 0 : linePos <= endPos; };
        while (keepGoing()) {
            auto label = getLabel(++labelIndex);
            label->setProperty("text", QString::number(lineValue, 'f', 2));
            placeLabel(label, linePos);

            const double pos = qRound(linePos);
            if (horiz) {
                a->majorLines.push_back(QLineF{ pos, (axisPos == Axis::Position::Bottom ? 0. : height()), pos, basePos + tickLength });
            } else {
                a->majorLines.push_back(QLineF{ (axisPos == Axis::Position::Right ? 0. : width()), pos, basePos + tickLength, pos });
            }

            double minorLineValue = lineValue + minorLinesLogicalDistance;
            double minorLinePos = linePos + minorLinesPixelDistance;
            lineValue += majorLinesLogicalDistance;
            linePos += majorLinesPixelDistance;

            int minorLine = 0;
            while (minorLineValue < lineValue) {
                const double pos = qRound(minorLinePos);
                if (horiz) {
                    a->minorLines.push_back(QLineF{ pos, basePos, pos, basePos + tickLength });
                } else {
                    a->minorLines.push_back(QLineF{ basePos, pos, basePos + tickLength, pos });
                }

                if (minorLine++ == 4) {
                    auto label = getLabel(++labelIndex);
                    label->setProperty("text", QString::number(minorLineValue, 'f', 2));

                    // check if the label would fit, otherwise don't show it
                    const double neededSpace = horiz ? label->implicitWidth() : label->implicitHeight();
                    if (fabs(majorLinesPixelDistance) > neededSpace * 2.5) {
                        placeLabel(label, minorLinePos);
                    } else {
                        --labelIndex;
                    }
                }

                minorLineValue += minorLinesLogicalDistance;
                minorLinePos += minorLinesPixelDistance;
            }
        }
        a->needsUpdate = true;
    }
}

QSGNode *ChartItem::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    if (!node) {
        node = new QSGNode;
        node->appendChildNode(new QSGTransformNode);
        node->appendChildNode(window()->createRectangleNode());
    }

    for (auto &a: m_axes) {
        if (!a->node) {
            a->node = new AxisNode(a.get());
            node->insertChildNodeAfter(a->node, node->firstChild());
        }
        a->node->update();
    }

    QMatrix4x4 matrix;
    matrix.translate(m_verticalMargin, m_horizontalMargin);
    auto plotsParentNode = node->firstChild();
    static_cast<QSGTransformNode *>(plotsParentNode)->setMatrix(matrix);

    for (auto p: m_plotsToInit) {
        plotsParentNode->appendChildNode(p->sgNode());
    }
    m_plotsToInit.clear();

    auto rect = mapRectToScene(QRectF(m_verticalMargin, m_horizontalMargin, width() - 2. * m_verticalMargin, height() - 2. * m_horizontalMargin)).toRect();

    for (auto p: m_plots) {
        p->update(window(), rect, window()->effectiveDevicePixelRatio(), m_paused);
    }

    auto rectNode = static_cast<QSGRectangleNode *>(node->lastChild());
    rectNode->setRect(m_zoomRect);
    rectNode->setColor(m_zoomRectColor);
    return node;
}

void ChartItem::wheelEvent(QWheelEvent *evt)
{
    double m = 1.1;
    if (evt->angleDelta().y() < 0) {
        m = 1. / m;
    }

    const auto chartRect = QRectF(m_verticalMargin, m_horizontalMargin,
                                  width() - 2. * m_verticalMargin,
                                  height() - 2. * m_horizontalMargin);
    if (chartRect.contains(evt->position())) {
        for (auto &a: m_axes) {
            a->zoom(m);
        }
        return;
    }

    for (auto &a: m_axes) {
        auto p = a->axis->position();
        switch (p) {
            case Axis::Position::Bottom: {
                if (QRectF(0, height() - m_verticalMargin, width(), m_verticalMargin).contains(evt->position())) {
                    a->zoom(m);
                }
                break;
            }
            case Axis::Position::Left: {
                if (QRectF(0, 0, m_verticalMargin, height()).contains(evt->position())) {
                    a->zoom(m);
                }
                break;
            }
            case Axis::Position::Top: {
                if (QRectF(0, 0, width(), m_horizontalMargin).contains(evt->position())) {
                    a->zoom(m);
                }
                break;
            }
            case Axis::Position::Right: {
                if (QRectF(width() - m_verticalMargin, 0, m_verticalMargin, height()).contains(evt->position())) {
                    a->zoom(m);
                }
                break;
            }
        }
    }
}

void ChartItem::mousePressEvent(QMouseEvent *evt)
{
    m_pressPos = evt->position();
    switch (evt->button()) {
        case Qt::MiddleButton: {
            m_panning = true;
            break;
        }
        case Qt::LeftButton: {
            m_drawingZoomSquare = true;
            m_zoomRect.setTopLeft(evt->position());
            m_zoomRect.setSize({});
            break;
        }
        default: {
            break;
        }
    }
    evt->accept();
}

void ChartItem::mouseMoveEvent(QMouseEvent *evt)
{
    auto dpos = evt->position() - m_pressPos;
    const auto dx = dpos.x() / (width() - 2. * m_verticalMargin);
    const auto dy = dpos.y() / (height() - 2. * m_horizontalMargin);

    if (m_panning) {
        for (auto &a: m_axes) {
            const auto range = a->axis->max() - a->axis->min();
            const auto p = a->axis->position();
            double d = range * (p == Axis::Position::Top || p == Axis::Position::Bottom ? dx : dy);

            if (a->isInverted()) {
                d = -d;
            }

            a->axis->setMin(a->axis->min() - d);
            a->axis->setMax(a->axis->max() - d);
        }
        m_pressPos = evt->position();
    }
    if (m_drawingZoomSquare) {
        m_zoomRect.setBottomRight(evt->position());
        update();
    }
}

void ChartItem::mouseReleaseEvent(QMouseEvent *evt)
{
    switch (evt->button()) {
        case Qt::MiddleButton: {
            m_panning = false;
            break;
        }
        case Qt::LeftButton: {
            m_zoomRect = sanitizeZoomRect(m_zoomRect);
            zoomIn(m_zoomRect);
            m_zoomHistory.push(m_zoomRect);

            m_drawingZoomSquare = false;
            m_zoomRect = {};
            update();
            break;
        }
        case Qt::RightButton: {
            if (!m_zoomHistory.empty()) {
                zoomOut(m_zoomHistory.top());
                m_zoomHistory.pop();
            }
            break;
        }
        default: {
            break;
        }
    }
}

void ChartItem::zoomIn(QRectF area)
{
    area = sanitizeZoomRect(area);
    if (area.isEmpty()) {
        return;
    }

    for (auto &a: m_axes) {
        const auto range = a->axis->max() - a->axis->min();
        const auto p = a->axis->position();

        const bool horiz = p == Axis::Position::Top || p == Axis::Position::Bottom;
        const double fullPixelSize = horiz ? (width() - 2. * m_verticalMargin) : (height() - 2. * m_horizontalMargin);

        double min = horiz ? area.x() - m_verticalMargin : area.y() - m_horizontalMargin;
        double max = horiz ? area.right() - m_verticalMargin : area.bottom() - m_horizontalMargin;
        if (a->isInverted()) {
            min = horiz ? width() - m_verticalMargin - area.right() : height() - m_horizontalMargin - area.bottom();
            max = horiz ? width() - m_verticalMargin - area.x() : height() - m_horizontalMargin - area.y();
        }

        a->axis->setMax(a->axis->min() + range * max / fullPixelSize);
        a->axis->setMin(a->axis->min() + range * min / fullPixelSize);
    }
}

void ChartItem::zoomOut(QRectF area)
{
    area = sanitizeZoomRect(area);
    if (area.isEmpty()) {
        return;
    }

    for (auto &a: m_axes) {
        const auto range = a->axis->max() - a->axis->min();
        const auto p = a->axis->position();

        const bool horiz = p == Axis::Position::Top || p == Axis::Position::Bottom;
        const double fullPixelSize = horiz ? area.width() : area.height();

        double min = horiz ? -area.x() + m_verticalMargin : -area.y() + m_horizontalMargin;
        double max = horiz ? width() - m_verticalMargin - area.x() : height() - 2. * m_horizontalMargin - area.y();
        if (a->isInverted()) {
            min = horiz ? m_horizontalMargin -(width() - area.right()) : m_horizontalMargin -(height() - area.bottom());
            max = horiz ? 0 : area.bottom() - m_horizontalMargin;
        }

        a->axis->setMax(a->axis->min() + range * max / fullPixelSize);
        a->axis->setMin(a->axis->min() + range * min / fullPixelSize);
    }
}

QRectF ChartItem::sanitizeZoomRect(QRectF rect)
{
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

}
