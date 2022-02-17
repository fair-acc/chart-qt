#include "defaultzoomhandler.h"

#include <QDebug>
#include <QWheelEvent>

#include "axis.h"
#include "chartitem.h"

namespace chart_qt {

DefaultZoomHandler::DefaultZoomHandler(QObject *parent)
    : QObject(parent) {
}

QQuickItem *DefaultZoomHandler::zoomRectangle() const {
    return m_zoomRectangle;
}

void DefaultZoomHandler::setZoomRectangle(QQuickItem *r) {
    if (m_zoomRectangle != r) {
        m_zoomRectangle = r;
        emit zoomRectangleChanged();

        m_zoomRectangle->setVisible(false);
    }
}

void DefaultZoomHandler::classBegin() {
}

void DefaultZoomHandler::componentComplete() {
    if (auto c = qobject_cast<ChartItem *>(parent())) {
        c->installEventFilter(this);
    }

    if (!m_zoomRectangle) {
        QQmlComponent           defaultZoomRectangle = QQmlComponent(qmlEngine(this));
        static const QByteArray qmlCode              = "import QtQuick 2.0\n Rectangle { color: \"#965ac8fa\" }";
        defaultZoomRectangle.setData(qmlCode, {});

        m_zoomRectangle = static_cast<QQuickItem *>(defaultZoomRectangle.create());
        m_zoomRectangle->setParentItem(chartItem());
        m_zoomRectangle->setParent(parent());
        m_zoomRectangle->setVisible(false);
    }
}

bool DefaultZoomHandler::eventFilter(QObject *object, QEvent *event) {
    switch (event->type()) {
    case QEvent::Wheel:
        wheelEvent(static_cast<QWheelEvent *>(event));
        return true;
    case QEvent::MouseButtonPress:
        mousePressEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseMove:
        mouseMoveEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::MouseButtonRelease:
        mouseReleaseEvent(static_cast<QMouseEvent *>(event));
        return true;
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        touchEvent(static_cast<QTouchEvent *>(event));
        return true;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

ChartItem *DefaultZoomHandler::chartItem() const {
    return static_cast<ChartItem *>(parent());
}

void DefaultZoomHandler::wheelEvent(QWheelEvent *evt) {
    double m = 1.1;
    if (evt->angleDelta().y() < 0) {
        m = 1. / m;
    }

    auto c    = chartItem();

    auto zoom = [&](Axis *axis) {
        auto       rect   = c->axisRect(axis);
        const auto p      = axis->position();
        const auto range  = axis->max() - axis->min();
        const bool horiz  = p == Axis::Position::Top || p == Axis::Position::Bottom;
        auto       crect  = c->contentRect();
        double     anchor = horiz ? (evt->position().x() - rect.x() - crect.x()) / crect.width() * range : (evt->position().y() - rect.y() - crect.y()) / crect.height() * range;

        if (axis->isRightToLeftOrBottomToTop()) {
            anchor = range - anchor;
        }
        axis->zoom(m, axis->min() + anchor);
    };

    const auto &axes = c->axes();
    if (c->contentRect().contains(evt->position())) {
        for (auto a : axes) {
            zoom(a);
        }
        return;
    }

    for (auto a : axes) {
        auto p = a->position();
        if (c->axisRect(a).contains(evt->position())) {
            zoom(a);
        }
    }
}

void DefaultZoomHandler::mousePressEvent(QMouseEvent *evt) {
    switch (evt->button()) {
    case Qt::MiddleButton: {
        startPanning(evt->position());
        break;
    }
    case Qt::LeftButton: {
        m_pinchPoints[0] = evt->position();
        m_zoomRectangle->setX(evt->position().x());
        m_zoomRectangle->setY(evt->position().y());
        m_zoomRectangle->setWidth(0);
        m_zoomRectangle->setHeight(0);
        m_zoomRectangle->setVisible(true);
        break;
    }
    default: {
        break;
    }
    }
    evt->accept();
}

void DefaultZoomHandler::mouseMoveEvent(QMouseEvent *evt) {
    if (!m_panningAxis.empty()) {
        pan(evt->position());
    }
    if (m_zoomRectangle->isVisible()) {
        m_pinchPoints[1] = evt->position();

        int x            = std::min(m_pinchPoints[0].x(), m_pinchPoints[1].x());
        int y            = std::min(m_pinchPoints[0].y(), m_pinchPoints[1].y());
        int w            = std::abs(m_pinchPoints[0].x() - m_pinchPoints[1].x());
        int h            = std::abs(m_pinchPoints[0].y() - m_pinchPoints[1].y());

        m_zoomRectangle->setX(x);
        m_zoomRectangle->setY(y);
        m_zoomRectangle->setWidth(w);
        m_zoomRectangle->setHeight(h);
    }
}

void DefaultZoomHandler::mouseReleaseEvent(QMouseEvent *evt) {
    switch (evt->button()) {
    case Qt::MiddleButton: {
        m_panningAxis.clear();
        break;
    }
    case Qt::LeftButton: {
        chartItem()->zoomIn(QRectF(m_zoomRectangle->x(), m_zoomRectangle->y(), m_zoomRectangle->width(), m_zoomRectangle->height()));
        m_zoomRectangle->setVisible(false);
        break;
    }
    case Qt::RightButton: {
        chartItem()->undoZoom();
        break;
    }
    default: {
        break;
    }
    }
}

void DefaultZoomHandler::touchEvent(QTouchEvent *evt) {
    if (evt->points().size() == 1) {
        if (evt->isBeginEvent()) {
            startPanning(evt->point(0).position());
        } else if (evt->isEndEvent()) {
            m_panningAxis.clear();
        } else {
            pan(evt->point(0).position());
        }
    } else {
        m_panningAxis.clear();
        if (evt->points().size() == 2) {
            const auto &p0 = evt->point(0).position();
            const auto &p1 = evt->point(1).position();
            if (evt->isBeginEvent()) {
                m_pinchPoints[0] = p0;
                m_pinchPoints[1] = p1;
            } else if (evt->isUpdateEvent()) {
                auto    center = (p0 + p1) / 2.;

                QPointF m      = { std::fabs(p0.x() - p1.x()) / std::fabs(m_pinchPoints[0].x() - m_pinchPoints[1].x()),
                    std::fabs(p0.y() - p1.y()) / std::fabs(m_pinchPoints[0].y() - m_pinchPoints[1].y()) };

                auto    c      = chartItem();
                auto    crect  = c->contentRect();
                for (auto &a : c->axes()) {
                    const auto p     = a->position();
                    const bool horiz = p == Axis::Position::Top || p == Axis::Position::Bottom;

                    auto       f     = horiz ? m.x() : m.y();
                    if (f <= 0 || !std::isnormal(f)) {
                        continue;
                    }

                    auto       rect   = c->axisRect(a);
                    const auto range  = a->max() - a->min();
                    double     anchor = horiz ? (center.x() - rect.x() - crect.x()) / crect.width() * range : (center.y() - rect.y() - crect.y()) / crect.height() * range;

                    if (a->isRightToLeftOrBottomToTop()) {
                        anchor = range - anchor;
                    }
                    a->zoom(f, a->min() + anchor);
                }

                m_pinchPoints[0] = p0;
                m_pinchPoints[1] = p1;
            }
        }
    }
}

void DefaultZoomHandler::startPanning(const QPointF &pos) {
    auto c = chartItem();
    if (c->contentRect().contains(pos)) {
        for (auto a : c->axes()) {
            m_panningAxis.push_back(a);
        }
    } else
        for (auto a : c->axes()) {
            if (c->axisRect(a).contains(pos)) {
                m_panningAxis.push_back(a);
            }
        }
    m_pressPos = pos;
}

void DefaultZoomHandler::pan(const QPointF &pos) {
    auto       c    = chartItem();
    auto       dpos = pos - m_pressPos;
    const auto dx   = dpos.x() / c->contentRect().width();
    const auto dy   = dpos.y() / c->contentRect().height();

    for (auto *a : m_panningAxis) {
        const auto range = a->max() - a->min();
        const auto p     = a->position();
        double     d     = range * (p == Axis::Position::Top || p == Axis::Position::Bottom ? dx : dy);

        if (a->isRightToLeftOrBottomToTop()) {
            d = -d;
        }

        a->setMin(a->min() - d);
        a->setMax(a->max() - d);
    }
    m_pressPos = pos;
}

} // namespace chart_qt
