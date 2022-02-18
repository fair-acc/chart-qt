#ifndef WATERFALLPLOT_H
#define WATERFALLPLOT_H

#include <QQmlEngine>

#include "plot.h"

namespace chart_qt {

class WaterfallPlot : public Plot {
    Q_OBJECT
    Q_PROPERTY(double gradientStart READ gradientStart WRITE setGradientStart NOTIFY gradientChanged)
    Q_PROPERTY(double gradientStop READ gradientStop WRITE setGradientStop NOTIFY gradientChanged)
    QML_ELEMENT
public:
    WaterfallPlot();

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
