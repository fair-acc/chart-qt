#ifndef WATERFALLPLOT_H
#define WATERFALLPLOT_H

#include "plot.h"

namespace chart_qt {

class WaterfallPlot : public Plot
{
    Q_OBJECT
    Q_PROPERTY(double gradientStart READ gradientStart WRITE setGradientStart NOTIFY gradientChanged)
    Q_PROPERTY(double gradientStop READ gradientStop WRITE setGradientStop NOTIFY gradientChanged)

public:

    double gradientStart() const;
    void setGradientStart(double g);

    double gradientStop() const;
    void setGradientStop(double g);

    QSGNode *sgNode() override;
    void update(QQuickWindow *window, double w, double h, bool paused) override;

signals:
    void gradientChanged();

private:
    class Node;

    size_t m_allocated = 0;
    Node *m_node = nullptr;

    double m_gradientStart = 0;
    double m_gradientStop = 0;
};

}

#endif
