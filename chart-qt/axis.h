#ifndef AXIS_H
#define AXIS_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQmlComponent>

class QQuickItem;

namespace chart_qt {

class Plot;

class Axis : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(double min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(double max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(Position position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(QQmlComponent *labelDelegate READ labelDelegate WRITE setLabelDelegate NOTIFY labelDelegateChanged)
    QML_ELEMENT
public:
    enum class Position
    {
        Bottom,
        Top,
        Right,
        Left,
    };
    Q_ENUM(Position);

    enum class Direction
    {
        LeftToRight = 0,
        BottomToTop = 0,
        RightToLeft = 1,
        TopToBottom = 1,
    };
    Q_ENUM(Direction)

    explicit Axis(QObject *parent = nullptr);

    double min() const;
    void setMin(double m);

    double max() const;
    void setMax(double m);

    Position position() const;
    void setPosition(Position p);

    Direction direction() const;
    void setDirection(Direction dir);

    QQmlComponent *labelDelegate() const;
    void setLabelDelegate(QQmlComponent *component);

    void classBegin() override;
    void componentComplete() override;

    QQuickItem *createLabel();

signals:
    void minChanged();
    void maxChanged();
    void positionChanged();
    void directionChanged();
    void labelDelegateChanged();

private:
    void createDefaultLabelDelegate();

    Position m_position = {};
    Direction m_direction = {};
    double m_min = 0;
    double m_max = 1;
    QQmlComponent *m_labelDelegate = nullptr;
    QQmlComponent *m_defaultLabelDelegate = nullptr;
};

}

#endif
