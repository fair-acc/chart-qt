#include "window.h"

#include <QMenuBar>
#include <QChartView>
#include <QValueAxis>

#include "xyplot.h"
#include "sindataset.h"

namespace chart_qt {

Window::Window()
{
    menuBar()->addMenu("File");

    auto chart = new QChartView(this);
//     setCentralWidget(QWidget::createWindowContainer(chart));
    setCentralWidget(chart);

    QValueAxis *yAxis = new QValueAxis(this);
    yAxis->setMin(-2);
    yAxis->setMax(2);
    chart->chart()->addAxis(yAxis, Qt::AlignLeft);

    QValueAxis *xAxis = new QValueAxis(this);
    xAxis->setMin(0);
    xAxis->setMax(70);
    chart->chart()->addAxis(xAxis, Qt::AlignBottom);


    auto dataset = new SinDataSet;

    auto plot = new XYPlot;
    plot->setDataSet(dataset);
    auto series = plot->series();
    chart->chart()->addSeries(series);
    series->attachAxis(xAxis);
    series->attachAxis(yAxis);
    // chart->addPlot(std::move(plot));
}

}
