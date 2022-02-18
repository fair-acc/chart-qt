#include "axis.h"

#include <QQmlEngine>
#include <QQuickItem>

#include "chartitem.h"

namespace chart_qt {

Axis::Axis(QObject *parent)
    : QObject(parent) {
}

double Axis::min() const {
    return _min;
}

void Axis::setMin(double m) {
    if (_min != m) {
        _min = m;
        emit minChanged();
    }
}

double Axis::max() const {
    return _max;
}

void Axis::setMax(double m) {
    if (_max != m) {
        _max = m;
        emit maxChanged();
    }
}

Axis::Position Axis::position() const {
    return _position;
}

void Axis::setPosition(Axis::Position p) {
    if (_position != p) {
        _position = p;
        emit positionChanged();
    }
}

Axis::Direction Axis::direction() const {
    return _direction;
}

void Axis::setDirection(Direction dir) {
    if (_direction != dir) {
        _direction = dir;
        emit directionChanged();
    }
}

QQmlComponent *Axis::labelDelegate() const {
    return _labelDelegate;
}

void Axis::setLabelDelegate(QQmlComponent *comp) {
    if (_labelDelegate != comp) {
        _labelDelegate = comp;
        emit labelDelegateChanged();
    }
}

void Axis::classBegin() {
}

void Axis::componentComplete() {
    if (auto *chart = qobject_cast<ChartItem *>(parent())) {
        chart->addAxis(this);
    }
}

void Axis::createDefaultLabelDelegate() {
    _defaultLabelDelegate           = new QQmlComponent(qmlEngine(this));
    static const QByteArray qmlCode = "import QtQuick 2.0\n Text{}";
    _defaultLabelDelegate->setData(qmlCode, {});
}

QQuickItem *Axis::createLabel() {
    QQmlComponent *delegate;
    QQmlContext   *context;
    if (!_labelDelegate) {
        if (!_defaultLabelDelegate) {
            createDefaultLabelDelegate();
        }
        delegate = _defaultLabelDelegate;
        context  = QQmlEngine::contextForObject(this);
    } else {
        delegate = _labelDelegate;
        context  = delegate->creationContext();
    }

    auto obj = qobject_cast<QQuickItem *>(delegate->beginCreate(context));
    if (!obj) {
        qWarning("Axis::labelDelegate does not create an Item, falling back to the default delegate");
        delegate->completeCreate();
        delete obj;
        setLabelDelegate(nullptr);

        // start over, will use the default delegate
        return createLabel();
    }

    obj->setParent(this);
    delegate->completeCreate();
    return obj;
}

void Axis::zoom(double m, double anchorPoint) {
    double max = anchorPoint + (_max - anchorPoint) / m;
    double min = anchorPoint + (_min - anchorPoint) / m;
    setMin(min);
    setMax(max);
}

bool Axis::isRightToLeftOrBottomToTop() const {
    const bool horiz = _position == Axis::Position::Top || _position == Axis::Position::Bottom;
    return (horiz && _direction == Axis::Direction::RightToLeft) || (!horiz && _direction == Axis::Direction::BottomToTop);
}

} // namespace chart_qt
