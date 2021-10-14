#include "chartwidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include "plot.h"
#include "dataset.h"

namespace chart_qt {

ChartWidget::ChartWidget(QWidget *parent)
//            : QOpenGLWindow()
            : QOpenGLWidget(parent)
{
    QSurfaceFormat fmt;
    fmt.setMajorVersion(4);
    fmt.setMinorVersion(1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
//     setFormat(fmt);
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::addPlot(std::unique_ptr<Plot> &&plot)
{
    m_plotsToInit.push_back(plot.get());
    m_plots.push_back(std::move(plot));
    update();
}

void ChartWidget::initializeGL()
{
    auto gl = QOpenGLContext::currentContext()->functions();

    gl->glClearColor(1, 1, 1, 1);
    initPlots();
}

void ChartWidget::paintGL()
{
    auto gl = QOpenGLContext::currentContext()->functions();

    gl->glClear(GL_COLOR_BUFFER_BIT);

    initPlots();
    for (auto p: m_plotsToUpdate) {
        p->updateData(0, 0);
    }
    m_plotsToUpdate.clear();

    for (auto &p: m_plots) {
        p->render();
    }
}

void ChartWidget::resizeGL(int w, int h)
{
    for (auto &p: m_plots) {
        p->handleResize(w, h);
    }
}

void ChartWidget::initPlots()
{
    for (auto p: m_plotsToInit) {
        p->initialize();
        p->handleResize(width(), height());
        if (auto ds = p->dataSet()) {
            p->updateData(0, ds->getDataCount());
        }
        connect(p, &Plot::update, this, [p, this]() { schedulePlotUpdate(p); });
    }
    m_plotsToInit.clear();
}

void ChartWidget::schedulePlotUpdate(Plot *plot)
{
    m_plotsToUpdate.push_back(plot);
    update();
}

}
