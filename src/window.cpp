#include "window.h"

#include "chartwidget.h"
#include "xyplot.h"
#include "sindataset.h"

namespace chart_qt {

Window::Window()
{
    auto chart = new ChartWidget(this);
    setCentralWidget(chart);

    auto dataset = new SinDataSet;


    auto plot = std::make_unique<XYPlot>();
    plot->setDataSet(dataset);
    chart->addPlot(std::move(plot));
}

}
