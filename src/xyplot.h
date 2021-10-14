#ifndef XYPLOT_H
#define XYPLOT_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "plot.h"

class QLineSeries;

namespace chart_qt {

class XYPlot : public Plot
{
public:
    XYPlot();

    void initialize() override;
    void updateData(int index, int count) override;
    void render() override;
    void handleResize(int width, int height) override;

    QAbstractSeries *series() const override;

private:
    void updateSeries();

    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_buffer;
    QOpenGLVertexArrayObject m_vao;
    size_t m_allocated = 0;

    QLineSeries *m_series;
};

}

#endif
