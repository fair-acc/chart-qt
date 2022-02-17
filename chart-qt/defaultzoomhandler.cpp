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
    return _zoomRectangle;
}

void DefaultZoomHandler::setZoomRectangle(QQuickItem *r) {
    if (_zoomRectangle != r) {
        _zoomRectangle = r;
        emit zoomRectangleChanged();

        _zoomRectangle->setVisible(false);
    }
}

void DefaultZoomHandler::classBegin() {
}

void DefaultZoomHandler::componentComplete() {
    if (auto c = qobject_cast<ChartItem *>(parent())) {
        c->installEventFilter(this);
    }

    if (!_zoomRectangle) {
        QQmlComponent           defaultZoomRectangle = QQmlComponent(qmlEngine(this));
        static const QByteArray qmlCode              = "import QtQuick 2.0\n Rectangle { color: \"#965ac8fa\" }";
        defaultZoomRectangle.setData(qmlCode, {});

        _zoomRectangle = static_cast<QQuickItem *>(defaultZoomRectangle.create());
        _zoomRectangle->setParentItem(chartItem());
        _zoomRectangle->setParent(parent());
        _zoomRectangle->setVisible(false);
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
        _pinchPoints[0] = evt->position();
        _zoomRectangle->setX(evt->position().x());
        _zoomRectangle->setY(evt->position().y());
        _zoomRectangle->setWidth(0);
        _zoomRectangle->setHeight(0);
        _zoomRectangle->setVisible(true);
        break;
    }
    default: {
        break;
    }
    }
    evt->accept();
}

void DefaultZoomHandler::mouseMoveEvent(QMouseEvent *evt) {
    if (!_panningAxis.empty()) {
        pan(evt->position());
    }
    if (_zoomRectangle->isVisible()) {
        _pinchPoints[1] = evt->position();

        int x           = std::min(_pinchPoints[0].x(), _pinchPoints[1].x());
        int y           = std::min(_pinchPoints[0].y(), _pinchPoints[1].y());
        int w           = std::abs(_pinchPoints[0].x() - _pinchPoints[1].x());
        int h           = std::abs(_pinchPoints[0].y() - _pinchPoints[1].y());

        _zoomRectangle->setX(x);
        _zoomRectangle->setY(y);
        _zoomRectangle->setWidth(w);
        _zoomRectangle->setHeight(h);
    }
}

void DefaultZoomHandler::mouseReleaseEvent(QMouseEvent *evt) {
    switch (evt->button()) {
    case Qt::MiddleButton: {
        _panningAxis.clear();
        break;
    }
    case Qt::LeftButton: {
        chartItem()->zoomIn(QRectF(_zoomRectangle->x(), _zoomRectangle->y(), _zoomRectangle->width(), _zoomRectangle->height()));
        _zoomRectangle->setVisible(false);
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
            _panningAxis.clear();
        } else {
            pan(evt->point(0).position());
        }
    } else {
        _panningAxis.clear();
        if (evt->points().size() == 2) {
            const auto &p0 = evt->point(0).position();
            const auto &p1 = evt->point(1).position();
            if (evt->isBeginEvent()) {
                _pinchPoints[0] = p0;
                _pinchPoints[1] = p1;
            } else if (evt->isUpdateEvent()) {
                auto    center = (p0 + p1) / 2.;

                QPointF m      = { std::fabs(p0.x() - p1.x()) / std::fabs(_pinchPoints[0].x() - _pinchPoints[1].x()),
                    std::fabs(p0.y() - p1.y()) / std::fabs(_pinchPoints[0].y() - _pinchPoints[1].y()) };

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

                _pinchPoints[0] = p0;
                _pinchPoints[1] = p1;
            }
        }
    }
}

void DefaultZoomHandler::startPanning(const QPointF &pos) {
    auto c = chartItem();
    if (c->contentRect().contains(pos)) {
        for (auto a : c->axes()) {
            _panningAxis.push_back(a);
        }
    } else
        for (auto a : c->axes()) {
            if (c->axisRect(a).contains(pos)) {
                _panningAxis.push_back(a);
            }
        }
    _pressPos = pos;
}

void DefaultZoomHandler::pan(const QPointF &pos) {
    auto       c    = chartItem();
    auto       dpos = pos - _pressPos;
    const auto dx   = dpos.x() / c->contentRect().width();
    const auto dy   = dpos.y() / c->contentRect().height();

    for (auto *a : _panningAxis) {
        const auto range = a->max() - a->min();
        const auto p     = a->position();
        double     d     = range * (p == Axis::Position::Top || p == Axis::Position::Bottom ? dx : dy);

        if (a->isRightToLeftOrBottomToTop()) {
            d = -d;
        }

        a->setMin(a->min() - d);
        a->setMax(a->max() - d);
    }
    _pressPos = pos;
}

} // namespace chart_qt
