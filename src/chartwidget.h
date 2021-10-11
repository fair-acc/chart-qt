#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <memory>

#include <QOpenGLWidget>

namespace chart_qt {

class Plot;

class ChartWidget : public QOpenGLWidget
{
public:
    explicit ChartWidget(QWidget *parent);
    ~ChartWidget();

    void addPlot(std::unique_ptr<Plot> &&plot);
    void initializeGL() final;
    void paintGL() final;
    void resizeGL(int w, int h) final;

private:
    void initPlots();
    void schedulePlotUpdate(Plot *plot);

    std::vector<std::unique_ptr<Plot>> m_plots;
    std::vector<Plot *> m_plotsToInit;
    std::vector<Plot *> m_plotsToUpdate;
};

}

#endif
