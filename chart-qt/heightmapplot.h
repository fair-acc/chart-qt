#ifndef HEIGHTMAPPLOT_H
#define HEIGHTMAPPLOT_H

#include <QQmlEngine>

#include "plot.h"

namespace chart_qt {

class HeightmapPlot : public Plot {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(double gradientStart READ gradientStart WRITE setGradientStart NOTIFY gradientChanged)
    Q_PROPERTY(double gradientStop READ gradientStop WRITE setGradientStop NOTIFY gradientChanged)
public:
    HeightmapPlot();

    double        gradientStart() const;
    void          setGradientStart(double g);

    double        gradientStop() const;
    void          setGradientStop(double g);

    void          update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused) override;

    PlotRenderer *renderer() override;

signals:
    void gradientChanged();

private:
    class Renderer;
    class Node;

    double    _gradientStart = 0;
    double    _gradientStop  = 0;
    Renderer *_renderer;
};

} // namespace chart_qt

#endif
