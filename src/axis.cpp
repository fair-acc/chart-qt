#include "axis.h"

#include <QQuickItem>
#include <QQmlEngine>

#include "chartitem.h"

namespace chart_qt {

Axis::Axis(QObject *parent)
    : QObject(parent)
{

}

double Axis::min() const
{
    return m_min;
}

void Axis::setMin(double m)
{
    if (m_min != m) {
        m_min = m;
        emit minChanged();
    }
}

double Axis::max() const
{
    return m_max;
}

void Axis::setMax(double m)
{
    if (m_max != m) {
        m_max = m;
        emit maxChanged();
    }
}

Axis::Position Axis::position() const
{
    return m_position;
}

void Axis::setPosition(Axis::Position p)
{
    if (m_position != p) {
        m_position = p;
        emit positionChanged();
    }
}

Axis::Direction Axis::direction() const
{
    return m_direction;
}

void Axis::setDirection(Direction dir)
{
    if (m_direction != dir) {
        m_direction = dir;
        emit directionChanged();
    }
}

QQmlComponent *Axis::labelDelegate() const
{
    return m_labelDelegate;
}

void Axis::setLabelDelegate(QQmlComponent *comp)
{
    if (m_labelDelegate != comp) {
        m_labelDelegate = comp;
        emit labelDelegateChanged();
    }
}

void Axis::classBegin()
{

}

void Axis::componentComplete()
{
    if (auto *chart = qobject_cast<ChartItem *>(parent())) {
        chart->addAxis(this);
    }
}

void Axis::createDefaultLabelDelegate()
{
    m_defaultLabelDelegate = new QQmlComponent(qmlEngine(this));
    static const QByteArray qmlCode = "import QtQuick 2.0\n Text{}";
    m_defaultLabelDelegate->setData(qmlCode, {});
}

QQuickItem *Axis::createLabel()
{
    QQmlComponent *delegate;
    QQmlContext *context;
    if (!m_labelDelegate) {
        if (!m_defaultLabelDelegate) {
            createDefaultLabelDelegate();
        }
        delegate = m_defaultLabelDelegate;
        context = QQmlEngine::contextForObject(this);
    } else {
        delegate = m_labelDelegate;
        context = delegate->creationContext();
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



}
