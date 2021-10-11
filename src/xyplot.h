#ifndef XYPLOT_H
#define XYPLOT_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include "plot.h"

namespace chart_qt {

class XYPlot : public Plot
{
public:

    void initialize() override;
    void updateData(int index, int count) override;
    void render() override;
    void handleResize(int width, int height) override;

private:
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_buffer;
    size_t m_allocated = 0;
};

}

#endif
