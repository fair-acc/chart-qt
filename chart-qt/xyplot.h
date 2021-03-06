#ifndef XYPLOT_H
#define XYPLOT_H

#include <QQmlEngine>

#include "plot.h"

namespace chart_qt {

class Node;

class XYPlot : public Plot {
    Q_OBJECT
    QML_ELEMENT
public:
    XYPlot();
    void          update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused) override;

    PlotRenderer *renderer() override;

private:
    class XYRenderer;
    XYRenderer *_renderer = nullptr;
};

} // namespace chart_qt

#endif
