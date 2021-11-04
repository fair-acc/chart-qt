#ifndef XYPLOT_H
#define XYPLOT_H

#include <QLineSeries>

#include "plot.h"

class QAbstractAxis;

namespace chart_qt {

class Node;

class XYPlot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QLineSeries *series MEMBER m_series)
public:
    XYPlot();

    void updateSeries();

private:
    QLineSeries *m_series;
};

}

#endif
